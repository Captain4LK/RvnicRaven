/*
RvnicRaven - data (de-)compression

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_COMPRESS_H_

#define _RVR_COMPRESS_H_

void  RvR_crush_compress(RvR_rw *in, RvR_rw *out, unsigned level);
void *RvR_crush_decompress(RvR_rw *in, int32_t *length);

#endif
