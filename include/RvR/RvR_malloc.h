/*
RvnicRaven - memory allocation

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_MALLOC_H_

#define _RVR_MALLOC_H_

typedef enum
{
   RVR_MALLOC_STATIC = 0,
   RVR_MALLOC_CACHE = 1,

   RVR_MALLOC_USR0 = 2,
   RVR_MALLOC_USR1 = 3,
   RVR_MALLOC_USR2 = 4,
   RVR_MALLOC_USR3 = 5,
}RvR_malloc_tag;

void  RvR_malloc_init(void *buffer, size_t size);
void *RvR_malloc(size_t size, const char *reason);
void *RvR_malloc_inane(size_t size);
void  RvR_free(void *ptr);
void *RvR_realloc(void *ptr, size_t size, const char *reason);
void *RvR_realloc_inane(void *ptr, size_t size);
void  RvR_malloc_report();
void *RvR_malloc_base();

void RvR_mem_tag(void *ptr, uint8_t tag);
void RvR_mem_cache(void *ptr, void **usr);

#endif
