#pragma once
#include <cassert>

namespace fx {

void debug_trace(const char* format, ...);

#if defined(NDEBUG) || defined(__GNUC__)
#if defined(_MSC_VER)
// conditional expression is constant
#pragma warning(disable : 4127)
#endif
#define DEBUG_TRACE(format, ...) \
    if (0) debug_trace(format, ##__VA_ARGS__)
#else
#define DEBUG_TRACE(format, ...) \
    debug_trace(format, ##__VA_ARGS__)
#endif

#if defined(_MSC_VER)
#define ALWAYS_TRACE(format, ...) \
    debug_trace(format, ##__VA_ARGS__)
#else
#define ALWAYS_TRACE(format, ...) \
    if (0) debug_trace(format, ##__VA_ARGS__)
#endif

#if defined(NDEBUG)
#define DEBUG_ASSERT(x)     ((void)0)
#define DEBUG_REQUIRE(x)    ((void)0)
#define DEBUG_ENSURE(x)     ((void)0)
#define DEBUG_INSIST(x)     ((void)0)
#define DEBUG_INVARIANT(x)  ((void)0)
#else
#define DEBUG_ASSERT(x)     assert(x)
#define DEBUG_REQUIRE(x)    DEBUG_ASSERT(x)
#define DEBUG_ENSURE(x)     DEBUG_ASSERT(x)
#define DEBUG_INSIST(x)     DEBUG_ASSERT(x)
#define DEBUG_INVARIANT(x)  DEBUG_ASSERT(x)
#endif

} // namespace fx
