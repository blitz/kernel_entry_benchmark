#pragma once

#include "types.hpp"

void putc(char c);

void put(const char *str);
void put(uint64_t v);

template <typename T>
void format(T first)
{
  put(first);
}

template <typename T, typename... R>
void format(T first, R... rest)
{
  format(first);
  format(rest...);
}
