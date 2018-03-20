#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>

#ifdef _MSC_VER
// conditional expression is constant
#pragma warning(disable : 4127)
// function 'xxx' marked as __forceinline not inlined
#pragma warning(disable : 4714)
#endif

namespace fx {

typedef unsigned char byte_t;
typedef byte_t* byte_ptr;
typedef const byte_t* byte_cptr;

typedef size_t index_t;
typedef size_t offset_t;
typedef size_t count_t;

namespace constants
{
    const uint16_t uint16_max = UINT16_C(0xFFFF);
    const uint32_t uint32_max = UINT32_C(0xFFFFFFFF);
    const uint64_t uint64_max = UINT64_C(0xFFFFFFFFFFFFFFFF);
} 

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#if defined(__GNUC__)
    // 4.5.2 -> 40502
    #define GCC_VERSION \
        (__GNUC__* 10000 + __GNUC_MINOR__* 100 + __GNUC_PATCHLEVEL__)
#endif

#if defined(_MSC_VER)
    #define ALWAYS_INLINE __forceinline
    #define NEVER_INLINE __declspec(noinline)
#elif defined(__GNUC__) && (GCC_VERSION >= 30100)
    #define ALWAYS_INLINE __attribute__ ((always_inline)) inline
    #define NEVER_INLINE __attribute__ ((noinline))
#else
    #define ALWAYS_INLINE inline
    #define NEVER_INLINE
#endif

#if defined(_MSC_VER)
    #define MSVC_ANY_TARGET
    #if !defined(_M_IA64)
        #if defined(_M_X64)
            #define MSVC_X86_64
        #elif defined(_M_IX86)
            #define MSVC_X86
        #endif
    #endif
#endif

#if defined(__GNUC__)    
    #define GNUC_ANY_TARGET
    #if defined(__i386__) || defined(__i486__) || \
            defined(__i586__) || defined(__i686__)
        #define GNUC_X86
    #elif defined(__x86_64__) || defined(__amd64__)
        #define GNUC_X86_64
    #elif defined(__arm__)
        #define GNUC_ARM
    #elif defined(__powerpc__) || defined(__ppc__)
        #define GNUC_PPC
    #elif defined(__powerpc64__) || defined(__ppc64__)
        #define GNUC_PPC64
    #elif defined(__mips__)
        #define GNUC_MIPS
    #elif defined(__sh__)
        #define GNUC_SH
    #endif
#endif

#define STATIC_ASSERT(x) static_assert((x), "static assertion failed")

} // namespace fx
