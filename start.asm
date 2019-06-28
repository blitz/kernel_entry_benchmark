; -*- Mode: nasm -*-

extern main

%define PAGE_SIZE 4096
%define PTE_P (1 << 0)
%define PTE_W (1 << 1)
%define PTE_U (1 << 2)
%define PTE_A (1 << 5)
%define PTE_D (1 << 6)
%define PTE_PS (1 << 7)
%define PTE_G (1 << 8)

%define PTE_DEFAULT (PTE_P | PTE_W | PTE_U | PTE_A | PTE_D | PTE_G)

%define IA32_EFER 0xC0000080
%define IA32_EFER_SCE 0x001
%define IA32_EFER_LME 0x100
%define IA32_EFER_NXE 0x800

%define CR0_PE (1 << 0)
%define CR0_MP (1 << 1)
%define CR0_TS (1 << 3)
%define CR0_WP (1 << 16)
%define CR0_PG (1 << 31)

%define CR4_PAE (1 << 5)

%define RING0_CODE_SELECTOR 0x08
%define RING0_DATA_SELECTOR 0x10

section .text.init progbits alloc exec nowrite
bits 32

_mbheader:
align 4
  dd 1BADB002h                  ; magic
  dd 3h                         ; features
  dd -(3h + 1BADB002h)          ; checksum

align 8
_long_gdt:
  dw _long_gdt_end - _long_gdt - 1
  dd _long_gdt
  dw 0
  dq 0x00a09b0000000000         ; Code
  dq 0x00a0930000000000         ; Data
_long_gdt_end:

global _start
_start:
  ; Construct 1:1 2MB mapping at 2MB
  mov dword [ptab_pml4], ptab_pdpt + PTE_DEFAULT
  mov dword [ptab_pdpt], ptab_pd   + PTE_DEFAULT
  mov dword [ptab_pd + 8], (1 << 21) | PTE_DEFAULT | PTE_PS

  ; Load page table
  mov eax, ptab_pml4
  mov cr3, eax

  ; Long mode initialization. See Intel SDM Vol. 3 Chapter 9.8.5.
  mov eax, CR4_PAE
  mov cr4, eax

  xor edx, edx
  mov eax, IA32_EFER_LME | IA32_EFER_NXE | IA32_EFER_SCE
  mov ecx, IA32_EFER
  wrmsr

  ; Enable paging.
  mov eax, CR0_PE | CR0_MP | CR0_WP | CR0_PG
  mov cr0, eax

  lgdt [_long_gdt]
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

  call main
  ud2

section .bss
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
