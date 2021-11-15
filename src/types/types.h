#pragma once
#include <stdint.h>
#include <stdio.h>
#include <cstddef>
#include <stdlib.h>
#include <assert.h>

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
typedef char byte;

#define U8Max 255
#define U16Max 65535
#define S32Min ((s32)0x80000000)
#define S32Max ((s32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX

#ifdef DEBUG

#ifdef _WIN32

// TODO(Brandon): Figure out windows equivalent debug break
#define DEBUG_BREAK()
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
#define ASSERT(condition, message)                                                                                                                                                                                    \
    if (!(condition))                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                 \
        fprintf(stderr, "Assertion '%s' failed.\n", message);                                                                                                                                                         \
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
#else
#define ASSERT(condition, message)
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
