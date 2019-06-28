; -*- Mode: nasm -*-

extern call_gate_entry
global asm_call_gate_entry

section .text

asm_call_gate_entry:
  call call_gate_entry
  o64 retf

