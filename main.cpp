#include <dt.hpp>
#include <io.hpp>
#include <selectors.hpp>
#include <x86-instructions.hpp>

extern "C" [[noreturn]] void main();
extern "C" gdt_desc gdt[];

gdt_desc gdt[] {
    {},
    gdt_desc::kern_code_desc(),
    gdt_desc::kern_data_desc(),
    gdt_desc::user_data_desc(),
    gdt_desc::user_code_desc(),

    // TSS placeholders
    {}, {}
};

tss tss;

void main()
{
  // Finishing GDT with entries that cannot be constructed without runtime code.
  gdt[array_size(gdt) - 2] = gdt_desc::tss_desc(&tss);
  gdt[array_size(gdt) - 1] = gdt_desc::tss_desc_hi(&tss);
  ltr(ring0_tss_selector);

  format("Hello World!");
  cli_hlt();
}
