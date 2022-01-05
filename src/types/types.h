#pragma once
#include <stdint.h>
#include <stdio.h>
#include <cstddef>
#include <type_traits>
#include <atomic>

typedef unsigned int uint;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
typedef float f32;
typedef double f64;
typedef unsigned char byte;
typedef const char *str;

// Atomic types.
typedef std::atomic<unsigned int> atomic_uint;
typedef std::atomic<uint64_t> atomic_u64;
typedef std::atomic<uint32_t> atomic_u32;
typedef std::atomic<uint16_t> atomic_u16;
typedef std::atomic<uint8_t> atomic_u8;
typedef std::atomic<int> atomic_int;
typedef std::atomic<int64_t> atomic_s64;
typedef std::atomic<int32_t> atomic_s32;
typedef std::atomic<int16_t> atomic_s16;
typedef std::atomic<int8_t> atomic_s8;

typedef std::atomic<bool> atomic_bool;
typedef std::atomic<char> atomic_char;
typedef std::atomic<signed char> atomic_schar;
typedef std::atomic<unsigned char> atomic_uchar;
typedef std::atomic<short> atomic_short;
typedef std::atomic<unsigned short> atomic_ushort;

typedef std::atomic<intptr_t> atomic_intptr_t;
typedef std::atomic<uintptr_t> atomic_uintptr_t;
typedef std::atomic<size_t> atomic_size_t;

using std::forward;
using std::move;

#define U8Max 255
#define U16Max 65535
#define S32Min ((s32)0x80000000)
#define S32Max ((s32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min (-FLT_MAX)

#ifdef _WIN32
#define VULTR_API extern "C" __declspec(dllexport)
#else
#define VULTR_API extern "C"
#endif

#ifdef DEBUG
#ifdef _WIN32
#include <Windows.h>
#define DEBUG_BREAK() DebugBreak()
#elif __linux__
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#define DEBUG_BREAK()
#endif

#else
#define DEBUG_BREAK()
#endif // DEBUG

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef DEBUG
#define ASSERT(condition, message, ...)                                                                                                                                                                               \
	if (!(condition))                                                                                                                                                                                                 \
	{                                                                                                                                                                                                                 \
		fprintf(stderr, "Assertion '");                                                                                                                                                                               \
		fprintf(stderr, message __VA_OPT__(, ) __VA_ARGS__);                                                                                                                                                          \
		fprintf(stderr, "' failed.\n");                                                                                                                                                                               \
		fprintf(stderr, "in %s, line %d\n", __FILE__, __LINE__);                                                                                                                                                      \
		DEBUG_BREAK();                                                                                                                                                                                                \
	}
// clang-format on
#else
#define ASSERT(condition, message, ...)
#endif

#ifdef _WIN32
// TODO(Brandon): Figure out windows production assertions.
#define PRODUCTION_ASSERT(condition, message, ...)
#elif __linux__
#define PRODUCTION_ASSERT(condition, message, ...)                                                                                                                                                                    \
	if (!(condition))                                                                                                                                                                                                 \
	{                                                                                                                                                                                                                 \
		fprintf(stderr, "Assertion '");                                                                                                                                                                               \
		fprintf(stderr, message __VA_OPT__(, ) __VA_ARGS__);                                                                                                                                                          \
		fprintf(stderr, "' failed.\n");                                                                                                                                                                               \
		fprintf(stderr, "in %s, line %d\n", __FILE__, __LINE__);                                                                                                                                                      \
		abort();                                                                                                                                                                                                      \
	}
#else
#define PRODUCTION_ASSERT(condition, message)
#endif

#ifdef _WIN32
// TODO(Brandon): Figure out windows assertions.
#define THROW(message)
#elif __linux__
#define THROW(message)                                                                                                                                                                                                \
	fprintf(stderr, "Assertion '%s' failed.\n", message);                                                                                                                                                             \
	fprintf(stderr, "in %s, line %d\n", __FILE__, __LINE__);                                                                                                                                                          \
	abort();
#else
#define PRODUCTION_ASSERT(condition, message)
#endif

#define NOT_IMPLEMENTED(message) PRODUCTION_ASSERT(false, message)

#define BIT_IS_HIGH(x, n) (((x) >> (n)) & 1UL)
#define BIT_IS_LOW(x, n) !BIT_IS_HIGH(x, n)

inline constexpr u64 Kilobyte(u64 val) { return val * 1024; }
inline constexpr u64 Megabyte(u64 val) { return Kilobyte(val) * 1024; }
inline constexpr u64 Gigabyte(u64 val) { return Megabyte(val) * 1024; }
inline constexpr u64 Terabyte(u64 val) { return Gigabyte(val) * 1024; }
