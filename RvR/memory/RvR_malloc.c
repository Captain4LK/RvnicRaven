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
#include "RvR/RvR_log.h"
#include "RvR/RvR_malloc.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct rvr_malloc_memory_node rvr_malloc_memory_node;

struct rvr_malloc_memory_node
{
   //negative size --> free block, not ideal since size_t should be used
   intptr_t size;
   rvr_malloc_memory_node *next;

   void **usr;

#if RvR_malloc_reason
   const char *reason;
#endif
};

/*typedef struct
{
   size_t block_size;
   void *addr;
   rvr_malloc_memory_node *sfirst, *slast;
}rvr_malloc_block_manager;*/
//-------------------------------------

//Variables
static int rvr_malloc_bmanage_total = 0;
//static rvr_malloc_block_manager rvr_malloc_bmanage = {0};

static rvr_malloc_memory_node *rvr_malloc_sfirst = NULL;
static rvr_malloc_memory_node *rvr_malloc_slast = NULL;
size_t rvr_malloc_size = 0;
void *rvr_malloc_addr = NULL;
//-------------------------------------

//Function prototypes
static size_t rvr_malloc_block_pointer_size(void *ptr);
//-------------------------------------

//Function implementations

void RvR_malloc_init(void *buffer, size_t size)
{
   RvR_error_check(buffer!=NULL, "RvR_malloc_init", "memory buffer is NULL\n");
   RvR_error_check(size!=0, "RvR_malloc_init", "memory buffer size is 0\n");

   if(rvr_malloc_bmanage_total)
   {
      RvR_log_line("RvR_malloc_init", "already initialized\n");
      return;
   }

   rvr_malloc_size = size;
   rvr_malloc_addr = buffer;

   rvr_malloc_sfirst = (rvr_malloc_memory_node *)(((char *)buffer));
   rvr_malloc_slast = rvr_malloc_sfirst;
   rvr_malloc_sfirst->size = -((intptr_t)size- (intptr_t)sizeof(rvr_malloc_memory_node));
   rvr_malloc_sfirst->next = NULL;

   //rvr_malloc_block_init(&rvr_malloc_bmanage, buffer, size);
   rvr_malloc_bmanage_total++;

   RvR_log_line("RvR_malloc_init", "allocated %zu bytes for allocator\n", size);

RvR_err:
   return;
}

void *RvR_malloc(size_t size, const char *reason)
{
   RvR_error_check(size!=0, "RvR_malloc", "0 byte allocation\n");

   if(!rvr_malloc_bmanage_total)
      return malloc(size);

   //Allign to pointer size
   size = (uintptr_t)(size + (sizeof(void *) - 1)) & (UINTPTR_MAX - (sizeof(void *) - 1));

   //TODO
   //void *mem = rvr_malloc_block_alloc(&rvr_malloc_bmanage, size, reason);

   rvr_malloc_memory_node *s = rvr_malloc_sfirst;
   if(s==NULL)
      return NULL;
   for(; s&&-s->size<size; s = s->next);
   if(s==NULL)
      return NULL;
   s->size = -s->size;

   if(s->size - size>(intptr_t)sizeof(rvr_malloc_memory_node) + (intptr_t)sizeof(void *))  //is there enough space to split the block?
   {
      rvr_malloc_memory_node *p = (rvr_malloc_memory_node *)((char *)s + sizeof(rvr_malloc_memory_node) + size);
      if(s==rvr_malloc_slast)
         rvr_malloc_slast = p;
      p->size = -(s->size - size - (intptr_t)sizeof(rvr_malloc_memory_node));
      p->next = s->next;
      s->next = p;
      s->size = size;
   }

#if RvR_malloc_reason
   s->reason = reason;
#endif

   void *mem = (void *)(((char *)s) + sizeof(rvr_malloc_memory_node));
   if(mem!=NULL)
      return mem;

   //TODO: try to free some unnecessary allocations in this case
   //      free currently unused textures?
   RvR_log_line("RvR_malloc", "allocation of size %zu failed, out of memory\n", size);
   RvR_malloc_report();

RvR_err:
   return NULL;
}

void *RvR_malloc_inane(size_t size)
{
   return RvR_malloc(size, "Lazy dev");
}

void *RvR_malloc_base()
{
   return rvr_malloc_addr;
}

void RvR_free(void *ptr)
{
   if(!rvr_malloc_bmanage_total)
   {
      free(ptr);
      return;
   }

   if(ptr>=(void *)rvr_malloc_sfirst)  //is the pointer in this block?
   {
      if(ptr<=(void *)rvr_malloc_slast)  //is it in static space?
      {
         //rvr_malloc_block_free(&rvr_malloc_bmanage, ptr);

         rvr_malloc_memory_node *o = (rvr_malloc_memory_node *)(((char *)ptr) - sizeof(rvr_malloc_memory_node)), *last = NULL;

         if(o->next&&o->next->size<0)   //see if we can add into next block
         {
            if(o->next==rvr_malloc_slast)
               rvr_malloc_slast = o;
            o->size += -o->next->size + sizeof(rvr_malloc_memory_node);
            o->next = o->next->next;
         }

         rvr_malloc_memory_node *n = rvr_malloc_sfirst;
         for(; n&&n!=o; n = n->next)
            last = n;

         if(last&&last->size<0)
         {
            if(o==rvr_malloc_slast)
               rvr_malloc_slast = last;
            last->next = o->next;
            last->size -= o->size + sizeof(rvr_malloc_memory_node);
         }
         else
         {
            o->size = -o->size;
         }

         return;
      }
   }

   RvR_log_line("RvR_free", "bad pointer\n");
}

void *RvR_realloc(void *ptr, size_t size, const char *reason)
{
   if(ptr==NULL)
      return RvR_malloc(size, reason);

   if(!rvr_malloc_bmanage_total)
   {
      void *d = realloc(ptr, size);
      return d;
   }

   if(size==0)
   {
      RvR_free(ptr);
      return NULL;
   }

   size_t old_size = 0;
   if(ptr>=(void *)rvr_malloc_sfirst &&
      ptr<=(void *)(((char *)rvr_malloc_sfirst) + rvr_malloc_size))
   {
      old_size = rvr_malloc_block_pointer_size(ptr);

      if(ptr<=(void *)rvr_malloc_slast)
      {
         void *nptr = RvR_malloc(size, reason);
         if(size>old_size)
            memcpy(nptr, ptr, old_size);
         else
            memcpy(nptr, ptr, size);

         RvR_free(ptr);
         //rvr_malloc_block_free(&rvr_malloc_bmanage, ptr);

         return nptr;
      }
   }

   RvR_log_line("RvR_realloc", "bad pointer\n");
   return NULL;
}

void *RvR_realloc_inane(void *ptr, size_t size)
{
   return RvR_realloc(ptr, size, "Lazy dev, realloc edition");
}

void RvR_malloc_report()
{
   if(!rvr_malloc_bmanage_total)
   {
      RvR_log_line("RvR_malloc_report", "using system allocator, memory report not possible\n");
      return;
   }

   RvR_log("************** Block size = %zu ***************\n", rvr_malloc_size);
   RvR_log("Index\tBase\t\t(Offset)\t      Size\tReason\n");
   rvr_malloc_memory_node * f = rvr_malloc_sfirst;
   size_t f_total = 0, a_total = 0;

   for(int i = 0; f; f = f->next, i++)
   {
      RvR_log("%4d\t%p\t(%10ld)\t%10" PRIdPTR "\t", i, (void *)f, ((char *)f - (char *)rvr_malloc_sfirst), f->size);
      if(f->size>0)
         a_total += f->size;
      else
         f_total += -f->size;

#if RvR_malloc_reason
      if(f->size>0)
         RvR_log("%s", f->reason);
      else
         RvR_log("FREE");
#endif

      RvR_log("\n");
   }

   RvR_log("**************** Block summary : %zu free, %zu allocated\n", f_total, a_total);
}

static size_t rvr_malloc_block_pointer_size(void *ptr)
{
   return ((rvr_malloc_memory_node *)(((char *)ptr) - sizeof(rvr_malloc_memory_node)))->size;
}
//-------------------------------------
