; -*- Mode: nasm -*-

extern _init_array_start, _init_array_end
extern main
extern gdt

%define PAGE_SIZE 4096
%define PTE_P (1 << 0)
%define PTE_W (1 << 1)
%define PTE_U (1 << 2)
%define PTE_A (1 << 5)
%define PTE_D (1 << 6)
%define PTE_PS (1 << 7)
%define PTE_G (1 << 8)

%define PTE_DEFAULT (PTE_P | PTE_W | PTE_U | PTE_A | PTE_D)

%define IA32_EFER 0xC0000080
%define IA32_EFER_SCE (1 << 0)
%define IA32_EFER_LME (1 << 8)
%define IA32_EFER_NXE (1 << 11)

%define CR0_PE (1 << 0)
%define CR0_MP (1 << 1)
%define CR0_TS (1 << 3)
%define CR0_NE (1 << 5)
%define CR0_WP (1 << 16)
%define CR0_PG (1 << 31)

%define CR4_PAE (1 << 5)
%define CR4_PGE (1 << 7)

%define RING0_CODE_SELECTOR 0x08
%define RING0_DATA_SELECTOR 0x10
%define GDT_SIZE 0x50

section .text.init progbits alloc exec nowrite
bits 32

_mbheader:
align 4
  dd 1BADB002h                  ; magic
  dd 3h                         ; features
  dd -(3h + 1BADB002h)          ; checksum

_gdtp:
  dw GDT_SIZE - 1
  dd gdt

global _start
_start:
  ; Construct 1:1 2MB mapping at 2MB
  mov dword [ptab_pml4], ptab_pdpt + PTE_DEFAULT ; PTE_G is reserved in PML4 on AMD.
  mov dword [ptab_pdpt], ptab_pd   + PTE_DEFAULT
  mov dword [ptab_pd + 8], (1 << 21) | PTE_DEFAULT | PTE_PS | PTE_G

  ; Load 64-bit GDT
  lgdt [_gdtp]

  ; Load page table
  mov eax, ptab_pml4
  mov cr3, eax

  ; Long mode initialization. See Intel SDM Vol. 3 Chapter 9.8.5.
  mov eax, CR4_PAE | CR4_PGE
  mov cr4, eax

  xor edx, edx
  mov eax, IA32_EFER_LME | IA32_EFER_NXE | IA32_EFER_SCE
  mov ecx, IA32_EFER
  wrmsr

  ; Enable paging and thus Long Mode.
  mov eax, cr0
  or eax, CR0_PG
  mov cr0, eax

  jmp RING0_CODE_SELECTOR:_start_long

section .text.longinit
bits 64
_start_long:

  mov eax, RING0_DATA_SELECTOR
  mov ss, eax
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax

  lea rsp, [rel kern_stack_end]

  mov rbx, _init_array_start
  mov rbp, _init_array_end

next_constructor:
  cmp rbx, rbp
  je done_with_contructors
  call [rbx]
  lea rbx, [rbx + 8]
  jmp next_constructor

done_with_contructors:
  call main
  ud2

section .bss
global kern_stack_end
  align PAGE_SIZE
kern_stack:
  resb PAGE_SIZE
kern_stack_end:

ptab_pml4:
  resb PAGE_SIZE
ptab_pdpt:
  resb PAGE_SIZE
ptab_pd:
  resb PAGE_SIZE
