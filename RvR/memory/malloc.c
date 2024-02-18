/*
RvnicRaven - memory allocation

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#if RVR_ASAN
#include "sanitizer/asan_interface.h"
#else
#define ASAN_POISON_MEMORY_REGION(a, b)
#define ASAN_UNPOISON_MEMORY_REGION(a, b)
#endif

#include "RvR/RvR_app.h"
#include "RvR/RvR_log.h"
#include "RvR/RvR_malloc.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct rvr_malloc_node rvr_malloc_node;

struct rvr_malloc_node
{
   size_t size;
   rvr_malloc_node *next;
   rvr_malloc_node *prev;

   void **usr;

   uint8_t tag;
   uint8_t pad0;
   uint8_t pad1;
   uint8_t pad2;

#if RvR_malloc_reason
   const char *reason;
#endif
};
//-------------------------------------

//Variables
static int rvr_malloc_init = 0;

static rvr_malloc_node *rvr_malloc_next = NULL;
static rvr_malloc_node *rvr_malloc_first = NULL;
size_t rvr_malloc_size = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_malloc_init(void *buffer, size_t size)
{
   RvR_error_check(!rvr_malloc_init, "RvR_malloc_init", "already initialized\n");
   RvR_error_check(buffer!=NULL, "RvR_malloc_init", "memory buffer is NULL\n");
   RvR_error_check(size!=0, "RvR_malloc_init", "memory buffer size is 0\n");

   rvr_malloc_first = buffer;
   rvr_malloc_first->size = size - sizeof(rvr_malloc_node);
   rvr_malloc_first->next = rvr_malloc_first;
   rvr_malloc_first->prev = rvr_malloc_first;
   rvr_malloc_next = rvr_malloc_first;
   rvr_malloc_size = size;

   rvr_malloc_init = 1;
   ASAN_POISON_MEMORY_REGION(rvr_malloc_first + 1, rvr_malloc_first->size);

   RvR_log_line("RvR_malloc_init", "allocated %zu bytes for allocator\n", size);

RvR_err:
   return;
}

void *RvR_malloc(size_t size, const char *reason)
{
   RvR_error_check(size!=0, "RvR_malloc", "0 byte allocation\n");

   if(!rvr_malloc_init)
      return malloc(size);

   //Align to pointer size
   size = (uintptr_t)(size + (sizeof(void *) - 1)) & (UINTPTR_MAX - (sizeof(void *) - 1));

   rvr_malloc_node *cur = rvr_malloc_next;
   if(cur->prev->tag==RVR_MALLOC_FREE)
      cur = cur->prev;

   rvr_malloc_node *first = cur;

   do
   {
      //Free cached blocks
      if(cur->tag==RVR_MALLOC_CACHE)
      {
         first = cur->prev;
         RvR_free(cur + 1);

         //Might have merged
         if(first->next==cur)
            first = first->next;
         else
            cur = first;
      }

      if(cur->tag==RVR_MALLOC_FREE&&cur->size>=size)
      {
         ASAN_UNPOISON_MEMORY_REGION(cur + 1, size);
         //Split block if possible
         if(cur->size - size>sizeof(rvr_malloc_node) + sizeof(void *))
         {
            rvr_malloc_node *p = (rvr_malloc_node *)((char *)cur + sizeof(rvr_malloc_node) + size);
            ASAN_UNPOISON_MEMORY_REGION(p, sizeof(*p));

            p->next = cur->next;
            p->next->prev = p;

            p->prev = cur;
            p->prev->next = p;

            p->size = cur->size - size - sizeof(rvr_malloc_node);
            cur->size = size;
            p->tag = RVR_MALLOC_FREE;
         }

         rvr_malloc_next = cur->next;

         cur->usr = NULL;
         cur->tag = RVR_MALLOC_STATIC;
         cur->reason = reason;

         return cur + 1;
      }

      cur = cur->next;
   }while(cur!=first);

   //Well, shit
   //At this point all cache has been freed, but we still don't have enough memory
   RvR_log_line("RvR_malloc", "allocation of size %zu failed, out of memory\n", size);
   RvR_malloc_report();

RvR_err:
   return NULL;
}

void *RvR_malloc_inane(size_t size)
{
   return RvR_malloc(size, "Lazy dev");
}

void RvR_free(void *ptr)
{
   if(!rvr_malloc_init)
   {
      free(ptr);
      return;
   }

   RvR_error_check(ptr, "RvR_free", "null pointer\n");

   rvr_malloc_node *n = (rvr_malloc_node *)(((char *)ptr) - sizeof(rvr_malloc_node));

   if(n->usr!=NULL)
      *n->usr = NULL;

   n->tag = RVR_MALLOC_FREE;
   ASAN_POISON_MEMORY_REGION(n + 1, n->size);

   //Merge with previous
   if(n!=rvr_malloc_first)
   {
      rvr_malloc_node *prev = n->prev;
      if(prev->tag==RVR_MALLOC_FREE)
      {
         if(rvr_malloc_next==n)
            rvr_malloc_next = prev;
         prev->next = n->next;
         prev->next->prev = prev;
         prev->size += n->size + sizeof(rvr_malloc_node);
         n = prev;

         ASAN_POISON_MEMORY_REGION(prev + 1, prev->size);
      }
   }

   //Merge with next
   rvr_malloc_node *next = n->next;
   if(next->tag==RVR_MALLOC_FREE&&next!=rvr_malloc_first)
   {
      if(rvr_malloc_next==next)
         rvr_malloc_next = n;
      n->next = next->next;
      n->next->prev = n;
      n->size += next->size + sizeof(rvr_malloc_node);
      ASAN_POISON_MEMORY_REGION(n + 1, n->size);
   }

RvR_err:
   return;
}

void *RvR_malloc_base()
{
   return rvr_malloc_first;
}

void *RvR_realloc(void *ptr, size_t size, const char *reason)
{
   if(ptr==NULL)
      return RvR_malloc(size, reason);

   if(!rvr_malloc_init)
   {
      void *d = realloc(ptr, size);
      return d;
   }

   if(size==0)
   {
      RvR_free(ptr);
      return NULL;
   }

   rvr_malloc_node *n = (rvr_malloc_node *)(((char *)ptr) - sizeof(rvr_malloc_node));
   size_t old_size = n->size;
   //printf("%zu %zu\n",old_size,size);
   void *nptr = RvR_malloc(size, reason);
   if(size>old_size)
      memcpy(nptr, ptr, old_size);
   else
      memcpy(nptr, ptr, size);

   if(n->usr)
   {
      rvr_malloc_node *np = (rvr_malloc_node *)(((char *)nptr) - sizeof(rvr_malloc_node));
      np->usr = n->usr;
      n->usr = NULL;
   }

   RvR_free(ptr);

   return nptr;
}

void *RvR_realloc_inane(void *ptr, size_t size)
{
   return RvR_realloc(ptr, size, "Lazy dev, realloc edition");
}

void RvR_malloc_report()
{
   if(!rvr_malloc_init)
   {
      RvR_log_line("RvR_malloc_report", "using system allocator, memory report not possible\n");
      return;
   }

   RvR_log("***************************** Block size = %zu ******************************\n", rvr_malloc_size);
   RvR_log("Index\tBase\t\t(Offset)\t      Size\tTag\tReason\n");
   rvr_malloc_node * f = rvr_malloc_first;
   size_t f_total = 0;
   size_t a_total = 0;
   size_t c_total = 0;

   int i = 0;
   do
   {
      if(f->tag!=RVR_MALLOC_FREE)
         RvR_log("%4d\t%p\t(%10ld)\t%10zu      %3" PRIu8 "\t", i, (void *)f, ((char *)f - (char *)rvr_malloc_first), f->size, f->tag);
      else
         RvR_log("%4d\t%p\t(%10ld)\t%10zu         \t", i, (void *)f, ((char *)f - (char *)rvr_malloc_first), f->size);

      if(f->tag==RVR_MALLOC_FREE)
         f_total += f->size;
      else if(f->tag==RVR_MALLOC_CACHE)
         c_total += f->size;
      else
         a_total += f->size;

#if RvR_malloc_reason
      if(f->tag!=RVR_MALLOC_FREE)
         RvR_log("%s", f->reason);
      else
         RvR_log("FREE");
#endif

      RvR_log("\n");

      i++;
      f = f->next;
   }while(f!=rvr_malloc_first);

   RvR_log("**************** Block summary : %zu free, %zu cached, %zu allocated\n", f_total, c_total, a_total);
}

void RvR_mem_tag_set(void *ptr, uint8_t tag)
{
   RvR_error_check(rvr_malloc_init, "RvR_mem_tag_set", "can't tag memory while using system allocator\n");
   RvR_error_check(ptr!=NULL, "RvR_mem_tag_set", "can't tag null\n");

   rvr_malloc_node *n = (rvr_malloc_node *)(((char *)ptr) - sizeof(rvr_malloc_node));
   n->tag = tag;

RvR_err:
   return;
}

void RvR_mem_usr_set(void *ptr, void **usr)
{
   RvR_error_check(rvr_malloc_init, "RvR_mem_usr_set", "can't set memory usr while using system allocator\n");
   RvR_error_check(ptr!=NULL, "RvR_mem_usr_set", "can't set usr of null pointer\n");

   rvr_malloc_node *n = (rvr_malloc_node *)(((char *)ptr) - sizeof(rvr_malloc_node));
   n->usr = usr;

RvR_err:
   return;
}
//-------------------------------------
