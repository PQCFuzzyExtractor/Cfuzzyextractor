#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    
    typedef uint32_t u32;
    typedef uint16_t u16;
    typedef uint8_t  u8;
    typedef uint64_t u64;

    #define GFP_KERNEL 0
    static inline void *kmalloc(size_t size, int flags) { return malloc(size); }
    static inline void kfree(const void *ptr) { free((void *)ptr); }

    static inline int fls(int x) {
        unsigned long index;
        #ifdef _MSC_VER
            if (_BitScanReverse(&index, (unsigned long)x)) return index + 1;
        #elif defined(__GNUC__)
            if (x != 0) return 32 - __builtin_clz(x);
        #endif
        return 0;
    }

    #ifdef _MSC_VER
        #define cpu_to_be32(x) _byteswap_ulong(x)
        #define be32_to_cpu(x) _byteswap_ulong(x)
    #elif defined(__GNUC__)
        #define cpu_to_be32(x) __builtin_bswap32(x)
        #define be32_to_cpu(x) __builtin_bswap32(x)
    #endif

    #ifndef ARRAY_SIZE
        #define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
    #endif
#else
    #include <strings.h> 
    #include <endian.h>
#endif

#endif // WIN_COMPAT_H