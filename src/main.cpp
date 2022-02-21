#include <dt.hpp>
#include <io.hpp>
#include <selectors.hpp>
#include <x86-instructions.hpp>
#include <msr.hpp>

extern "C" void asm_call_gate_entry();
extern "C" void asm_int_gate_entry();
extern "C" void asm_syscall_entry();
extern "C" void asm_sysenter_entry();
extern "C" char kern_stack_end[];

extern "C" [[noreturn]] void main();
extern "C" void call_gate_entry();

extern "C" gdt_desc gdt[];


alignas(64) char user_stack[128];

gdt_desc gdt[] {
    {},
    gdt_desc::kern_code_desc(),
    gdt_desc::kern_data_desc(),
    gdt_desc::user_code_desc(),
    gdt_desc::user_data_desc(),
    gdt_desc::user_code_desc(), // Duplicated because of syscall/sysret selector requirements

    // TSS placeholders
    {}, {},

    // Call gate placeholders
    {}, {},
};

tss tss;

static const uint8_t INT_GATE_VECTOR {32};

static idt_desc idt[256];

[[noreturn]] static void exit_via_retf(uint64_t user_rip)
{
  asm volatile ("push %[user_ss]\n"
		"push %[user_rsp]\n"
		"push %[user_cs]\n"
		"push %[user_rip]\n"
		// Address width for far ret is 32-bit even in 64-bit mode.
		"rex.W lret\n"
		:
		: [user_ss]  "ri" (static_cast<uint64_t>(ring3_data_selector)),
		  [user_rsp] "ri" (reinterpret_cast<uintptr_t>(user_stack) + sizeof(user_stack)),
		  [user_cs]  "ri" (static_cast<uint64_t>(ring3_code_selector)),
		  [user_rip] "ri" (user_rip));

  __builtin_unreachable();
}

static void do_gate_call()
{
  struct farp {
    uint32_t offset;
    uint16_t sel;
  } __packed;

  static const farp gate { 0, ring3_call_selector };
  // Adding a rex.W prefix to the far call, makes the far pointer have
  // a 64-bit offset on Intel, but has no effect on AMD.
  asm volatile ("lcall *%[far_ptr]\n" :: [far_ptr] "m" (gate));
}

static void do_int()
{
  asm volatile ("int %[vec]\n" :: [vec] "i" (INT_GATE_VECTOR));
}

static void do_syscall()
{
  asm volatile ("syscall\n" ::: "rcx", "r11");
}

static void do_sysenter()
{
  asm volatile ("mov %%rsp, %%rcx\n"
		"lea 1f(%%rip), %%rdx\n"
		"sysenter\n"
		"1: \n"
		::: "rcx", "rdx");
}

static void measure(const char *name, void (*fn)())
{
  const int measure_rounds = 128 * 1024;
  const int warmup_rounds = 32 * 1024;

  for (int warm_up = warmup_rounds; warm_up != 0; warm_up--) {
    fn();
  }

  uint64_t start = rdtsc();
  for (int rounds = measure_rounds; rounds != 0; rounds--) {
    fn();
  }
  uint64_t end = rdtsc();

  format(name, ",", (end - start) / measure_rounds, "\n");
}

[[noreturn]] static void user_measure()
{
  measure("Interrupt Gate", [] () { do_int(); });
  measure("Call Gate", [] () { do_gate_call(); });
  measure("Syscall", [] () { do_syscall(); });

  // This only works on Intel.
  if (is_vendor_intel(query_cpuid(0))) {
    measure("Sysenter", [] () { do_sysenter(); });
  } else {
    format("# Only Intel supports sysenter in Long Mode, skipping.\n");
  }

  // We're done.
  outbi<0x64>(0xFE);            // PCI reset on qemu
  cli_hlt();                    // This will triple fault
  __builtin_unreachable();
}

static void allow_user_io()
{
  // Set IOPL to 3.
  asm volatile ("pushfq\n"
		"or %0, (%%rsp)\n"
		"popfq\n"
		:
		: "r" (3 << 12));
}

static void init_syscall()
{
  wrmsr(IA32_LSTAR, reinterpret_cast<uintptr_t>(asm_syscall_entry));

  static_assert(ring0_code_selector + 0x8 == ring0_data_selector,
		"Ring0 code and data selectors do not match SYSRET layout");

  static_assert(ring3_data_selector + 0x8 == ring3_code_selector,
		"Ring0 code and data selectors do not match SYSRET layout");

  wrmsr(IA32_STAR, ((uint64_t)ring0_code_selector << 32) | (uint64_t)(ring3_code_selector - 0x10) << 48);
  wrmsr(IA32_FMASK, 0x700);     // Clear TF/DF/IF on system call entry
}

static void init_sysenter()
{
  static_assert(ring0_code_selector + 0x8 == ring0_data_selector,
		"Ring0 code and data selectors do not match SYSCALL layout");

  static_assert(ring0_code_selector + 0x10 + 3 == ring3_code_selector_sysenter,
		"Ring0 code and ring3 code selectors do not match SYSCALL layout");

  static_assert(ring0_code_selector + 0x18 + 3 == ring3_data_selector,
		"Ring0 code and ring3 data selectors do not match SYSCALL layout");

  wrmsr(IA32_SYSENTER_CS, ring0_code_selector);
  wrmsr(IA32_SYSENTER_ESP, reinterpret_cast<uintptr_t>(kern_stack_end));
  wrmsr(IA32_SYSENTER_EIP, reinterpret_cast<uintptr_t>(asm_sysenter_entry));
}

void main()
{
  // Finishing GDT with entries that cannot be constructed without runtime code.
  gdt[array_size(gdt) - 4] = gdt_desc::tss_desc(&tss);
  gdt[array_size(gdt) - 3] = gdt_desc::tss_desc_hi(&tss);

  gdt[array_size(gdt) - 2] = gdt_desc::call_gate(ring0_code_selector, reinterpret_cast<uintptr_t>(&asm_call_gate_entry), 3);
  gdt[array_size(gdt) - 1] = gdt_desc::call_gate_hi(reinterpret_cast<uintptr_t>(&asm_call_gate_entry));

  tss.rsp[0] = reinterpret_cast<uintptr_t>(kern_stack_end);
  ltr(ring0_tss_selector);

  // Prepare interrupt and call gate entry points
  idt[INT_GATE_VECTOR]  = idt_desc::interrupt_gate(ring0_code_selector, reinterpret_cast<uintptr_t>(&asm_int_gate_entry), 0, 3);
  lidt(idt);

  allow_user_io();
  init_syscall();
  init_sysenter();

  exit_via_retf(reinterpret_cast<uintptr_t>(&user_measure));

  cli_hlt();
}
