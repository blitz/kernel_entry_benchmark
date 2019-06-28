OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH("i386:x86-64")
ENTRY(_start)
EXTERN(_start)

PHDRS {
      lo_rwx PT_LOAD FILEHDR PHDRS FLAGS (7); /* RWX */
}

SECTIONS
{
      ENTRY(_start)

      /* 2 MB */
      .text (0x200000 + SIZEOF_HEADERS) :
      {
        *(.text.init)
        *(.text .text.*)
      } : lo_rwx

      .data :
      {
        . = ALIGN(8);
        _init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array))
        _init_array_end = .;
        *(.rodata .rodata.*);
        *(.data .data.*);
      } : lo_rwx

      .bss :
      {
        *(.bss .bss.*);
      } : lo_rwx

      /DISCARD/ :
      {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
      }
}
