; -*- Mode: nasm -*-

extern call_gate_entry, int_gate_entry
global asm_call_gate_entry, asm_int_gate_entry

section .text.asm_call_gate_entry

align 16
asm_call_gate_entry:
  call call_gate_entry
  o64 retf

align 16
asm_int_gate_entry:
  mov rdi, rsp
  call int_gate_entry
  iretq
