; -*- Mode: nasm -*-

extern call_gate_entry, int_gate_entry, syscall_entry
extern kern_stack_end

global asm_call_gate_entry, asm_int_gate_entry, asm_syscall_entry

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

align 16
asm_syscall_entry:
  mov rsp, kern_stack_end

  ; Need to canonicalize RCX (RIP) to avoid userspace triggering an exception in the kernel on the wrong stack.
  mov rax, 0x00007FFFFFFFFFFF
  and rcx, rax

  push rcx
  push r11

  call syscall_entry

  pop r11
  pop rcx

  o64 sysret
