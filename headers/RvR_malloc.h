#ifndef _RVR_MALLOC_H_

/*
   RvnicRaven - memory allocator

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

/*
   To create implementation (the function definitions) add
      #define RVR_MALLOC_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _RVR_MALLOC_H_

void  RvR_malloc_init(void *buffer, size_t size);
void *RvR_malloc(size_t size);
void  RvR_free(void *ptr);
void *RvR_realloc(void *ptr, size_t size);
void  RvR_malloc_report();
void *RvR_malloc_base();

#endif

#ifdef RVR_MALLOC_IMPLEMENTATION
#ifndef RVR_MALLOC_IMPLEMENTATION_ONCE
#define RVR_MALLOC_IMPLEMENTATION_ONCE

#ifndef RvR_log
#define RvR_log(...) while(0)
#endif

#ifndef RvR_error_fail
#define RvR_error_fail(w, ...) do { goto RvR_err; } while(0)
#endif

#ifndef RvR_error_check
#define RvR_error_check(X, w, ...) do { if(!(X)) RvR_error_fail(w, __VA_ARGS__); } while(0)
#endif

#ifndef RvR_log_line
#define RvR_log_line(w, ...) while(0)
#endif

typedef struct rvr_malloc_memory_node rvr_malloc_memory_node;

struct rvr_malloc_memory_node
{
   int32_t size;
   rvr_malloc_memory_node *next;
};

typedef struct
{
   int32_t block_size;
   void *addr;
   rvr_malloc_memory_node *sfirst, *slast;
}rvr_malloc_block_manager;

static int rvr_malloc_bmanage_total = 0;
static int rvr_malloc_instance = 0;
static rvr_malloc_block_manager rvr_malloc_bmanage = {0};

static void  rvr_malloc_block_init(rvr_malloc_block_manager *b, void *block, long block_size);
static void *rvr_malloc_block_alloc(rvr_malloc_block_manager *b, int32_t size);
static void  rvr_malloc_block_free(rvr_malloc_block_manager *b, void *ptr);
static long  rvr_malloc_block_pointer_size(void *ptr);
static void  rvr_malloc_block_report(rvr_malloc_block_manager *b);

void RvR_malloc_init(void *buffer, size_t size)
{
   RvR_error_check(buffer!=NULL, "RvR_malloc_init", "memory buffer is NULL\n");
   RvR_error_check(size!=0, "RvR_malloc_init", "memory buffer size is 0\n");

   if(rvr_malloc_bmanage_total)
   {
      RvR_log_line("RvR_malloc_init", "already initialized\n");
      return;
   }

   rvr_malloc_block_init(&rvr_malloc_bmanage, buffer, size);
   rvr_malloc_bmanage_total++;
   RvR_log_line("RvR_malloc_init", "allocated %zu bytes for allocator\n", size);

RvR_err:
   return;
}

void *RvR_malloc(size_t size)
{
   RvR_error_check(size!=0, "RvR_malloc", "tried to malloc 0 bytes\n");

   if(!rvr_malloc_bmanage_total)
   {
      return malloc(size);
   }

   rvr_malloc_instance++;
   if(rvr_malloc_instance==-1)
      RvR_log_line("RvR_malloc", "mem break\n");

   size = (size + 3) & (0xffffffff - 3);

   void *mem = rvr_malloc_block_alloc(&rvr_malloc_bmanage, size);
   if(mem!=NULL)
      return mem;

   //TODO: try to free some unnecessary allocations in this case
   //      free currently unused textures?
   RvR_log_line("RvR_malloc", "allocation of size %zu failed, out of memory\n", size);
   RvR_malloc_report();

RvR_err:
   return NULL;
}

void *RvR_malloc_base()
{
   return rvr_malloc_bmanage.addr;
}

void RvR_free(void *ptr)
{
   if(!rvr_malloc_bmanage_total)
   {
      free(ptr);
      return;
   }

   if(ptr>=(void *)rvr_malloc_bmanage.sfirst)  //is the pointer in this block?
   {
      if(ptr<=(void *)rvr_malloc_bmanage.slast)  //is it in static space?
      {
         rvr_malloc_block_free(&rvr_malloc_bmanage, ptr);
         return;
      }
   }

   RvR_log_line("RvR_free", "bad pointer\n");
}

void *RvR_realloc(void *ptr, size_t size)
{
   if(ptr==NULL)
      return RvR_malloc(size);

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

   int32_t old_size = 0;
   if(ptr>=(void *)rvr_malloc_bmanage.sfirst &&
      ptr<=(void *)(((char *)rvr_malloc_bmanage.sfirst) + rvr_malloc_bmanage.block_size))
   {
      old_size = rvr_malloc_block_pointer_size(ptr);

      if(ptr<=(void *)rvr_malloc_bmanage.slast)
      {
         void *nptr = RvR_malloc(size);
         if((int32_t)size>old_size)
            memcpy(nptr, ptr, old_size);
         else
            memcpy(nptr, ptr, size);

         rvr_malloc_block_free(&rvr_malloc_bmanage, ptr);

         return nptr;
      }
   }

   RvR_log_line("RvR_malloc", "bad pointer\n");
   return NULL;
}

void RvR_malloc_report()
{
   if(!rvr_malloc_bmanage_total)
   {
      RvR_log_line("RvR_malloc_report", "using system allocator, memory report not possible\n");
      return;
   }

   rvr_malloc_block_report(&rvr_malloc_bmanage);
}

static void rvr_malloc_block_init(rvr_malloc_block_manager *b, void *block, long block_size)
{
   b->block_size = block_size;
   b->addr = block;

   b->sfirst = (rvr_malloc_memory_node *)(((char *)block));
   b->slast = b->sfirst;
   b->sfirst->size = -(block_size - (int32_t)sizeof(rvr_malloc_memory_node));
   b->sfirst->next = NULL;
}

static void *rvr_malloc_block_alloc(rvr_malloc_block_manager *b, int32_t size)
{
   rvr_malloc_memory_node *s = b->sfirst;
   if(s==NULL)
      return NULL;
   for(; s&&-s->size<size; s = s->next);
   if(s==NULL)
      return NULL;
   s->size = -s->size;

   if(s->size - size>(int32_t)sizeof(rvr_malloc_memory_node) + 4)  //is there enough space to split the block?
   {
      rvr_malloc_memory_node *p = (rvr_malloc_memory_node *)((char *)s + sizeof(rvr_malloc_memory_node) + size);
      if(s==b->slast)
         b->slast = p;
      p->size = -(s->size - size - (int32_t)sizeof(rvr_malloc_memory_node));
      p->next = s->next;
      s->next = p;
      s->size = size;
   }

   return (void *)(((char *)s) + sizeof(rvr_malloc_memory_node));
}

static void rvr_malloc_block_free(rvr_malloc_block_manager *b, void *ptr)
{
   rvr_malloc_memory_node *o = (rvr_malloc_memory_node *)(((char *)ptr) - sizeof(rvr_malloc_memory_node)), *last = NULL;

   if(o->next&&o->next->size<0)   //see if we can add into next block
   {
      if(o->next==b->slast)
         b->slast = o;
      o->size += -o->next->size + sizeof(rvr_malloc_memory_node);
      o->next = o->next->next;
   }

   rvr_malloc_memory_node *n = b->sfirst;
   for(; n&&n!=o; n = n->next)
      last = n;

   if(last&&last->size<0)
   {
      if(o==b->slast)
         b->slast = last;
      last->next = o->next;
      last->size -= o->size + sizeof(rvr_malloc_memory_node);
   }
   else
   {
      o->size = -o->size;
   }
}

static long rvr_malloc_block_pointer_size(void *ptr)
{
   return ((rvr_malloc_memory_node *)(((char *)ptr) - sizeof(rvr_malloc_memory_node)))->size;
}

static void rvr_malloc_block_report(rvr_malloc_block_manager *b)
{
   RvR_log("************** Block size = %d ***************\n", b->block_size);
   RvR_log("Index\tBase\t\t(Offset)\t      Size\n");
   int i = 0;
   rvr_malloc_memory_node * f = b->sfirst;
   int32_t f_total = 0, a_total = 0;

   for(; f; f = f->next, i++)
   {
      RvR_log("%4d\t%p\t(%10ld)\t%10d", i, (void *)f, ((char *)f - (char *)b->sfirst), f->size);
      if(f->size>0)
      {
         a_total += f->size;
      }
      else
      {
         f_total += -f->size;
      }

      RvR_log("\n");
   }

   RvR_log("**************** Block summary : %d free, %d allocated\n", f_total, a_total);
}

#endif
#endif
