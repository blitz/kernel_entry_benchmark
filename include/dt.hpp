// x86_64 CPU data structures and helper functions

#pragma once

#include "types.hpp"
#include "x86-instructions.hpp"

// Task State Segment
struct tss {
  uint32_t reserved0 = 0;
  uint64_t rsp[3] {};
  uint32_t reserved1 = 0;
  uint32_t reserved2 = 0;
  uint64_t ist[7] {};
  uint32_t reserved3 = 0;
  uint32_t reserved4 = 0;
  uint16_t reserved5 = 0;
  uint16_t iopb_offset = 0;
} __packed;

// A descriptor for the Global Descriptor Table
struct gdt_desc {
  uint16_t limit_lo = 0;
  uint16_t base_lo = 0;
  uint8_t  base_mid = 0;
  uint8_t  type_dpl = 0;
  uint8_t  limit_flags = 0;
  uint8_t  base_hi = 0;

  constexpr gdt_desc() = default;

  constexpr void set_base(uint64_t base)
  {
    base_lo = base & 0xFFFF;
    base_mid = (base >> 16) & 0xFF;
    base_hi = (base >> 24) & 0xFF;
  }

  static gdt_desc tss_desc(tss const *base)
  {
    gdt_desc t;

    static_assert(sizeof(tss) < 0x10000U, "TSS size broken");

    t.limit_lo = sizeof(tss) & 0xFFFF;
    t.type_dpl = 0b10001001;
    t.set_base((uintptr_t)base);
    return t;
  }

  static gdt_desc tss_desc_hi(tss const *base)
  {
    union bare_access {
      gdt_desc t;
      uint32_t dword[2];

      bare_access() {}
    } b;

    b.dword[0] = reinterpret_cast<uintptr_t>(base) >> 32;
    b.dword[1] = 0;

    return b.t;
  }

  static gdt_desc call_gate(uint16_t sel_, uint64_t offset_, uint8_t dpl)
  {
    gdt_desc i;

    i.limit_lo = offset_ & 0xFFFF;
    i.base_lo = sel_;
    i.type_dpl = 0b10001100 | (dpl << 5);

    i.limit_flags = (offset_ >> 16) & 0xFF;
    i.base_hi     = (offset_ >> 24) & 0xFF;

    return i;
  }

  static gdt_desc call_gate_hi(uint64_t offset_)
  {
    gdt_desc i;

    union bare_access {
      gdt_desc t;
      uint32_t dword[2];

      bare_access() {}
    } b;

    b.dword[0] = offset_ >> 32;
    b.dword[1] = 0;

    return b.t;
  }

  constexpr static gdt_desc kern_code_desc()
  {
    gdt_desc t;
    t.type_dpl = 0b10011011;
    t.limit_flags = 0b10100000;
    return t;
  }

  constexpr static gdt_desc kern_data_desc()
  {
    gdt_desc t;
    t.type_dpl = 0b10010011;
    t.limit_flags = 0b10100000;
    return t;
  }

  constexpr static gdt_desc user_code_desc()
  {
    gdt_desc t;
    t.type_dpl = 0b11111011;
    t.limit_flags = 0b10100000;
    return t;
  }

  constexpr static gdt_desc user_data_desc()
  {
    gdt_desc t;
    t.type_dpl = 0b11110011;
    t.limit_flags = 0b10100000;
    return t;
  }
};

static_assert(sizeof(gdt_desc) == 8, "GDT structure broken");

// Interrupt descriptor table descriptor
struct idt_desc {
  uint16_t offset_lo = 0;
  uint16_t selector = 0;
  uint16_t flags = 0;
  uint16_t offset_hi = 0;
  uint32_t offset_hi2 = 0;
  uint32_t reserved = 0;

  idt_desc() = default;

  void set_offset(uint64_t offset_)
  {
    offset_lo = offset_ & 0xFFFF;
    offset_hi = (offset_ >> 16) & 0xFFFF;
    offset_hi2 = offset_ >> 32;
  }

  static idt_desc interrupt_gate(uint16_t sel_, uint64_t offset_, uint8_t ist, uint8_t dpl)
  {
    idt_desc i;
    i.set_offset(offset_);
    i.selector = sel_;
    i.flags = ist | (14 /* type */ << 8) | (dpl << 13) | (1 << 15 /* Present */);
    return i;
  }
};

// Descriptor table register (GDTR/LDTR)
template <typename DESC>
struct dtr {
  uint16_t length;
  DESC const *descp;

  template <size_t N>
  dtr(DESC const (&descp_)[N])
    : length(N * sizeof(DESC) - 1),
      descp(descp_)
  {}
} __packed;

inline void lidt(dtr<idt_desc> const &idtr)
{
  asm volatile ("lidt %0" :: "m" (idtr));
}

inline void lgdt(dtr<gdt_desc> const &gdtr)
{
  asm volatile ("lgdt %0" :: "m" (gdtr));
}

inline void ltr(uint16_t selector)
{
  asm volatile ("ltr %0" :: "r" ((uint16_t)selector));
}
