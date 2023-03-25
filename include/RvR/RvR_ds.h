/*
RvnicRaven - dynamic datastructures

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_DS_H_

#define _RVR_DS_H_

//Dynamic array
//------------------------------------
#define RvR_array_push(a, o) (RvR_array_maygrow(a, 1), (a)[RvR_array_header(a)->length++] = (o))
#define RvR_array_length(a) ((a)!=NULL?RvR_array_header(a)->length:0)
#define RvR_array_length_set(a, n) do { if((a)==NULL) RvR_array_grow(a, n, 0); else RvR_array_maygrow(a, n - RvR_array_header(a)->length); RvR_array_header(a)->length = n; }while(0)
#define RvR_array_free(a) ((a)!=NULL?(RvR_free(a - RvR_array_header_offset(a)), 0):0)

//Internal do not call/use yourself
typedef struct
{
   size_t length;
   size_t size;
}RvR_aheader;
#define RvR_array_header(a) ((RvR_aheader *)(a - RvR_array_header_offset(a)))
#define RvR_array_header_offset(a) ((sizeof(RvR_aheader) + sizeof(*a) - 1) / sizeof(*a))
#define RvR_array_maygrow(a, n) (((a)==NULL||RvR_array_header(a)->length + n>RvR_array_header(a)->size)?(RvR_array_grow(a, n, 0), 0):0)
#define RvR_array_grow(a, n, min) ((a) = RvR_array_grow_internal(a, sizeof(*a), n, min))
void *RvR_array_grow_internal(void *old, size_t size, size_t grow, size_t min);
//------------------------------------

#endif
