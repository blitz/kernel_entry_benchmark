#include <io.hpp>
#include <x86-instructions.hpp>

void putc(char c)
{
  outbi<0xe9>(c);
}

void put(const char *str)
{
  // Qemu debugcon
  for (;*str != 0; str++)
    putc(*str);
}

void put(uint64_t v)
{
  char digits[32] {};

  // Points at last character. We have to keep a '\0' at the end.
  char *cur = &digits[31];

  do {
    *(--cur) = v % 10 + '0';
    v = v / 10;
  } while (v);

  put(cur);
}

const char *hex(uint64_t v)
{
  bool skip_leading_zeroes = true;
  static char digits[2 /* 0x */ + 16 /* hex digits */ + 1 /* \0 */];
  char *cur = &digits[0];

  *(cur++) = '0';
  *(cur++) = 'x';

  for (int i = sizeof(v)*8 - 4; i >= 0; i -= 4) {
    int nibble = (v >> i) & 0xF;

    // Don't print leading zeroes.
    if (nibble == 0 and i != 0 and skip_leading_zeroes)
      continue;
    else
      skip_leading_zeroes = false;

    *(cur++) = nibble + (nibble < 0xa ? '0' : 'a' - 0xa);
  }

  *(cur++) = 0;
  return &digits[0];
}
