; -*- Mode: nasm -*-

extern call_gate_entry, int_gate_entry, syscall_entry, sysenter_entry
extern kern_stack_end

global asm_call_gate_entry, asm_int_gate_entry, asm_syscall_entry, asm_sysenter_entry

section .text.asm_call_gate_entry

align 16
asm_call_gate_entry:
  o64 retf

align 16
asm_int_gate_entry:
  iretq

align 16
asm_syscall_entry:
  ; The hardware sets RIP and clobbers RCX/R11.
  o64 sysret

align 16
asm_sysenter_entry:
  ; Hardware sets RIP and RSP. Userspace has to give us RIP and userspace
  ; RSP to jump back via RDX/RCX.
  o64 sysexit
