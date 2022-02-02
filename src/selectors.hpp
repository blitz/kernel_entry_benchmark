#pragma once

#include "types.hpp"

const uint16_t ring0_code_selector = 0x8;
const uint16_t ring0_data_selector = 0x10;
const uint16_t ring3_code_selector_sysenter = 0x1b;
const uint16_t ring3_data_selector = 0x23;
const uint16_t ring3_code_selector = 0x2b;
const uint16_t ring0_tss_selector = 0x30;
const uint16_t ring3_call_selector = 0x43;

inline bool is_cpl0_selector(uint16_t sel)
{
  return (sel & 0x3) == 0;
}
