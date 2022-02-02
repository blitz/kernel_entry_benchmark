#pragma once

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned;
using uint64_t = unsigned long long;

#ifdef __x86_64__
using size_t = unsigned long;
#elif defined(__i386__)
using size_t = unsigned int;
#else
# error Unknown platform
#endif

using uintptr_t = unsigned long;

#define __packed __attribute__((packed))

inline bool likely(bool b) { return __builtin_expect(b, 1); }
inline bool unlikely(bool b) { return __builtin_expect(b, 0); }

template <typename T, size_t SIZE>
constexpr size_t array_size(T(&)[SIZE]) { return SIZE; }

#define offsetof(type, member)  __builtin_offsetof (type, member)

inline void *operator new(size_t, void *p) { return p; }
