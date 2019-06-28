#include <dt.hpp>
#include <io.hpp>
#include <selectors.hpp>
#include <x86-instructions.hpp>

extern "C" void asm_call_gate_entry();
extern "C" char kern_stack_end[];

extern "C" [[noreturn]] void main();
extern "C" void call_gate_entry();

extern "C" gdt_desc gdt[];


alignas(64) char user_stack[128];

gdt_desc gdt[] {
    {},
    gdt_desc::kern_code_desc(),
    gdt_desc::kern_data_desc(),
    gdt_desc::user_data_desc(),
    gdt_desc::user_code_desc(),

    // TSS placeholders
    {}, {},

    // Call gate placeholders
    {}, {},
};

tss tss;

static const uint8_t INT_GATE_VECTOR {32};

static idt_desc idt[256];

static void int_gate_entry()
{
  format("interrupt!\n");
  cli_hlt();
}

extern "C" void call_gate_entry()
{
  // Do nothing and return.
}

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

static void do_gate_call(uint16_t selector)
{
  struct farp {
    uint64_t offset;
    uint16_t sel;
  } __packed;

  static uint64_t saved_rbp;

  const farp gate { 0, ring3_call_selector };
  asm volatile ("mov %%rbp, %[saved_rbp]\n"
                "rex.W lcall *%[far_ptr]\n"
                "mov %[saved_rbp], %%rbp\n"
                : [saved_rbp] "=m" (saved_rbp)
                : [far_ptr] "m" (gate)
                : "rax", "rcx", "rdx", "rbx",
                  "rbp", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "r12",
                  "r13", "r14", "r15");
}

[[noreturn]] static void user_do_call_gate()
{
  format("Starting call gate measurement...\n");

  for (int warm_up = 32; warm_up != 0; warm_up--) {
    do_gate_call(ring3_call_selector);
  }

  const int repeat = 16 * 1024;
  uint64_t start = rdtsc();
  for (int rounds = repeat; rounds != 0; rounds--) {
    do_gate_call(ring3_call_selector);
  }
  uint64_t end = rdtsc();

  format("Call gate roundtrip (cycles): ", (end - start) / repeat, "\n");

  cli_hlt();
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
  idt[INT_GATE_VECTOR]  = idt_desc::interrupt_gate(ring0_code_selector, reinterpret_cast<uintptr_t>(&int_gate_entry), 0, 3);
  lidt(idt);

  allow_user_io();
  exit_via_retf(reinterpret_cast<uintptr_t>(&user_do_call_gate));

  cli_hlt();
}
