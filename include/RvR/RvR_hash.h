/*
RvnicRaven - hashes

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_HASH_H_

#define _RVR_HASH_H_

#define RVR_HASH_FNV32_INIT (UINT32_C(0x811c9dc5))
#define RVR_HASH_FNV64_INIT (UINT64_C(0xcbf29ce484222325))

uint64_t RvR_fnv64a(const char *str);
uint64_t RvR_fnv64a_str(const char *str, uint64_t hval);
uint64_t RvR_fnv64a_buf(const void *buf, size_t len, uint64_t hval);
uint32_t RvR_fnv32a(const char *str);
uint32_t RvR_fnv32a_str(const char *str, uint32_t hval);
uint32_t RvR_fnv32a_buf(const void *buf, size_t len, uint32_t hval);

#endif
