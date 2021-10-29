#pragma once
#include <string.h>

// Generic equality
template <typename T>
bool generic_equals(T first, T second)
{
}
//
// Default compare functions
//
template <>
inline bool generic_equals<const char *>(const char *first, const char *second)
{
    return strcmp(first, second) == 0;
}
