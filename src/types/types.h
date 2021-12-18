#pragma once
#include <stdint.h>
#include <stdio.h>
#include <cstddef>
#include <stdlib.h>
#include <assert.h>
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

#define U8Max 255
#define U16Max 65535
#define S32Min ((s32)0x80000000)
#define S32Max ((s32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX


#ifdef _WIN32
#define VULTR_API extern "C" __declspec(dllexport)
#else 
#define VULTR_API
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
#endif // Platform

#else
#define DEBUG_BREAK()
#endif // DEBUG

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef DEBUG
// clang-format off
#define ASSERT(condition, message, ...)                                                                                                                                                                               \
    if (!(condition))                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                 \
        fprintf(stderr, "Assertion '");                                                                                                                                                                                \
        fprintf(stderr, message __VA_OPT__(,) __VA_ARGS__);                                                                                                                                                                     \
        fprintf(stderr, "' failed.\n");                                                                                                                                                                                \
        fprintf(stderr, "in %s, line %d\n", __FILE__, __LINE__);                                                                                                                                                      \
        fprintf(stderr, "Press (I)gnore / (D)ebug / (A)bort:\n");                                                                                                                                                     \
        char input = getchar();                                                                                                                                                                                       \
        switch (input)                                                                                                                                                                                                \
        {                                                                                                                                                                                                             \
            case 'I':                                                                                                                                                                                                 \
                printf("Ignoring assertion...\n");                                                                                                                                                                    \
                break;                                                                                                                                                                                                \
            case 'D':                                                                                                                                                                                                 \
                printf("Debugging...\n");                                                                                                                                                                             \
                DEBUG_BREAK();                                                                                                                                                                                        \
                break;                                                                                                                                                                                                \
            case 'A':                                                                                                                                                                                                 \
            default:                                                                                                                                                                                                  \
                printf("Aborting.\n");                                                                                                                                                                                \
                abort();                                                                                                                                                                                              \
                break;                                                                                                                                                                                                \
        }                                                                                                                                                                                                             \
    }
// clang-format on
#else
#define ASSERT(condition, message, ...)
#endif

#ifdef _WIN32
// TODO(Brandon): Figure out windows production assertions.
#define PRODUCTION_ASSERT(condition, message)
#elif __linux__
#define PRODUCTION_ASSERT(condition, message)                                                                                                                                                                         \
    if (!(condition))                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                 \
        fprintf(stderr, "Assertion '%s' failed.\n", message);                                                                                                                                                         \
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

struct Buffer
{
    u64 count = 0;
    u8 *data  = nullptr;

    Buffer()        = default;
    Buffer &operator=(const Buffer &other) = delete;

    ~Buffer()
    {
        if (data != nullptr)
        {
            // TODO(Brandon): Replace with custom allocator.
            // free(data);
        }
    }
};

typedef Buffer String;

inline u64 str_len(const char *string)
{
    u32 count = 0;
    if (string != nullptr)
    {
        while (*string++)
        {
            count++;
        }
    }
    return count;
}

inline u64 str_len(const String &string) { return string.count; }

inline String str_new(const char *string)
{
    String result;
    result.count = str_len(string);
    // TODO(Brandon): Replace with custom allocator.
    // result.data  = static_cast<u8 *>(malloc(sizeof(u8) * result.count));

    return result;
}

inline void str_free(String string)
{
    if (string.data != nullptr)
    {
        // TODO(Brandon): Replace with custom allocator.
        // free(string.data);
    }
}
