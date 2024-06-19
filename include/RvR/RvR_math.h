/*
RvnicRaven - general math utilities

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_MATH_H_

#define _RVR_MATH_H_

#include <stdint.h>

#define RvR_min(a, b) ((a)<(b)?(a):(b))
#define RvR_max(a, b) ((a)>(b)?(a):(b))
#define RvR_abs(a) ((a)<0?-(a):(a))
#define RvR_non_zero(a) ((a) + ((a)==0))
#define RvR_clamp(a, min, max) (RvR_max((min), RvR_min((max), (a))))

#define RvR_sign(a) ((a)>=0?1:-1)
#define RvR_sign_equal(a, b) (RvR_sign(a)==RvR_sign(b))

//count leading zeros
//clz(0) = bitwidth
//TODO(Captain4LK): generic versions are fairly new, try to fallback on non generic ones?
#if defined __has_builtin && __has_builtin (__builtin_clzg)
#define RvR_clz8(a) (__builtin_clzg((uint8_t)(a),8))
#define RvR_clz16(a) (__builtin_clzg((uint16_t)(a),16))
#define RvR_clz32(a) (__builtin_clzg((uint32_t)(a),32))
#define RvR_clz64(a) (__builtin_clzg((uint64_t)(a),64))
#else
#define RvR_clz8(a) RvR_clz8_gen(a);
#define RvR_clz16(a) RvR_clz16_gen(a);
#define RvR_clz32(a) RvR_clz32_gen(a);
#define RvR_clz64(a) RvR_clz64_gen(a);
#endif

int RvR_clz8_gen(uint8_t a);
int RvR_clz16_gen(uint16_t a);
int RvR_clz32_gen(uint32_t a);
int RvR_clz64_gen(uint64_t a);

inline uint32_t RvR_log2(uint32_t a) { if(a==0) return 0; return 31-RvR_clz32(a); }

#endif
