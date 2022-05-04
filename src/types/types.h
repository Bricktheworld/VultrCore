#pragma once
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <cstddef>
#include <type_traits>
#include <atomic>
#include <string.h>
#include <execinfo.h>
#include <sys/wait.h>
#include <sys/prctl.h>

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
#include <execinfo.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#define DEBUG_BREAK()
#endif

#else
#define DEBUG_BREAK()
#endif // DEBUG

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#if 0
#define PRINT_STACKTRACE()                                                                                                                                                                                            \
	{                                                                                                                                                                                                                 \
		char pid_buf[30];                                                                                                                                                                                             \
		sprintf(pid_buf, "%d", getpid());                                                                                                                                                                             \
		char name_buf[512];                                                                                                                                                                                           \
		name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;                                                                                                                                                      \
		prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);                                                                                                                                                           \
		int child_pid = fork();                                                                                                                                                                                       \
		if (!child_pid)                                                                                                                                                                                               \
		{                                                                                                                                                                                                             \
			dup2(2, 1);                                                                                                                                                                                               \
			execl("/usr/bin/gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, nullptr);                                                                                                  \
			abort();                                                                                                                                                                                                  \
		}                                                                                                                                                                                                             \
		else                                                                                                                                                                                                          \
		{                                                                                                                                                                                                             \
			waitpid(child_pid, nullptr, 0);                                                                                                                                                                           \
		}                                                                                                                                                                                                             \
	}
#endif

#ifdef DEBUG
#define ASSERT(condition, message, ...)                                                                                                                                                                               \
	if (!(condition))                                                                                                                                                                                                 \
	{                                                                                                                                                                                                                 \
		fprintf(stderr, "Assertion '");                                                                                                                                                                               \
		fprintf(stderr, message __VA_OPT__(, ) __VA_ARGS__);                                                                                                                                                          \
		fprintf(stderr, "' failed.\n");                                                                                                                                                                               \
		fprintf(stderr, "in %s, line %d\n", __FILE__, __LINE__);                                                                                                                                                      \
		assert(false);                                                                                                                                                                                                \
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
		assert(false);                                                                                                                                                                                                \
	}
#else
#define PRODUCTION_ASSERT(condition, message)
#endif

#ifdef _WIN32
// TODO(Brandon): Figure out windows assertions.
#define THROW(message)
#elif __linux__
#define THROW(message, ...)                                                                                                                                                                                           \
	{                                                                                                                                                                                                                 \
		fprintf(stderr, "Error was thrown '");                                                                                                                                                                        \
		fprintf(stderr, message __VA_OPT__(, ) __VA_ARGS__);                                                                                                                                                          \
		fprintf(stderr, "' failed.\n");                                                                                                                                                                               \
		fprintf(stderr, "in %s, line %d\n", __FILE__, __LINE__);                                                                                                                                                      \
		assert(false);                                                                                                                                                                                                \
	}
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
