#pragma once

#include "types.hpp"

// Wait forever.
[[noreturn]] inline void cli_hlt()
{
  while (true)
    asm volatile ("cli ; hlt");
}

// Immediate 8-bit OUT operation
template <int PORT>
inline void outbi(uint8_t data)
{
  static_assert(PORT >= 0 and PORT < 0x100, "Port is out of range");
  asm volatile ("outb %%al, %0" :: "N" (PORT), "a" (data));
}

inline void outl(uint16_t port, uint32_t value)
{
  asm volatile ("outl %0, %1" :: "a" (value), "d" (port));
}

inline uint32_t inl(uint16_t port)
{
  uint32_t v;
  asm volatile ("inl %1, %0" : "=a" (v) : "d" (port));
  return v;
}

inline uint64_t get_cr2()
{
  uint64_t v;
  asm volatile ("mov %%cr2, %0" : "=r" (v));
  return v;
}

inline void wrmsr(uint32_t sel, uint64_t val)
{
  asm volatile ("wrmsr"
                :
                : "c" (sel),
                  "a" ((uint32_t)val),
                  "d" ((uint32_t)(val >> 32)));
}

inline uint64_t rdmsr(uint32_t sel)
{
  uint32_t lo, hi;
  asm volatile ("rdmsr" : "=d" (hi), "=a" (lo) : "c" (sel));
  return (uint64_t)hi << 32 | lo;
}

inline uint64_t rdtsc()
{
  uint32_t lo, hi;
  asm volatile ("rdtsc" : "=d" (hi), "=a" (lo));
  return (uint64_t)hi << 32 | lo;
}

inline void pause()
{
  asm volatile ("pause");
}

// FPU handling

enum : uint64_t {
  XCR0_FPU = 1UL << 0,
  XCR0_SSE = 1UL << 1,
  XCR0_AVX = 1UL << 2,
};

inline void set_xcr0(uint64_t v)
{
  asm volatile ("xsetbv" :: "d" ((uint32_t)(v >> 32)), "a" ((uint32_t)v), "c" (0));
}

inline uint64_t get_xcr0()
{
  uint32_t lo, hi;
  asm volatile ("xgetbv" : "=d" (hi), "=a" (lo) : "c" (0));
  return (uint64_t)hi << 32 | lo;
}

// CPUID

struct cpuid_result {
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
};

inline cpuid_result query_cpuid(uint32_t eax, uint32_t ecx = 0)
{
  cpuid_result res;

  asm ("cpuid"
       : "=a" (res.eax), "=c" (res.ecx), "=d" (res.edx), "=b" (res.ebx)
       : "a" (eax), "c" (ecx));

  return res;
}

inline bool is_vendor_intel(cpuid_result const &res)
{
  return
    res.ebx == 0x756e6547 &&
    res.ecx == 0x6c65746e &&
    res.edx == 0x49656e69;
}

inline bool is_vendor_amd(cpuid_result const &res)
{
  return
    res.ebx == 0x68747541 &&
    res.ecx == 0x444d4163 &&
    res.edx == 0x69746e65;
}
