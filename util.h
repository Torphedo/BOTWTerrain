#pragma once
#include <common/int.h>

static void console_pause() {
#ifdef PLATFORM_WINDOWS
    system("pause");
#endif
}

/// @brief Interleave the bits of 2 values
///
/// Where 'y' is a bit from [y] and 'x' is a bit from [x], the resulting bits are:
/// yxyxyxyxyxyxyxyxyxyxyxyxyxyxyxyx
static u32 interleave16(u16 x, u16 y) {
    u32 result = 0;

    for (u32 i = 0; i < sizeof(x) * 8; i+= 2) {
        result |= ((u32)(x & 1)) << i;
        x >>= 1; // Cut off bottom bit
    }
    for (u32 i = 1; i < sizeof(y) * 8; i+= 2) {
        result |= ((u32)(y & 1)) << i;
        y >>= 1; // Cut off bottom bit
    }
    
    return result;
}

