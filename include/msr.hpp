#pragma once

#include "types.hpp"

enum : uint32_t {
  IA32_SYSENTER_CS = 0x174U,
  IA32_STAR = 0xC0000081U,
  IA32_LSTAR = 0xC0000082U,
  IA32_FMASK = 0xC0000084U,
  IA32_FS_BASE = 0xC0000100U,
};
