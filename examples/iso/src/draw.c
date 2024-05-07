/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "grp.h"
#include "draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs

typedef struct Span Span;

struct Span
{
   int x0;
   int x1;
   Span *next;
   Span **prev_next;
};
//-------------------------------------

//Variables
static Span *spans[RVR_YRES_MAX];
static Span *span_pool = NULL;

static int alloc = 0;
static int freed = 0;
//-------------------------------------

//Function prototypes
static Span *span_new();
static void span_free(Span *s);
static void span_add(Span **tree, Span *s, uint8_t *src, int y);
static void span_draw(int x0, int x1, uint8_t *src, int y);
//-------------------------------------

//Function implementations

void draw_begin()
{
   RvR_render_clear(0);
   for(int i = 0;i<RvR_yres();i++)
   {
      span_free(spans[i]);
      spans[i] = NULL;
   }

   printf("Allocated: %d; Freed: %d\n",alloc,freed);
   alloc = 0;
   freed = 0;
}

void draw_end()
{
}

void draw_map()
{
   srand(1);
   for(int z = 0;z<2;z++)
   {
      for(int y = 32;y>=0;y--)
      {
         for(int x = 0;x<32;x++)
         {
            if(rand()&1)
               continue;
            int sx = x*16+y*16;
            //int sy = -8*x+8*y;
            int sy = z*12-8*x+8*y;
            GRP_block *grp = grp_block_get(0);
            RvR_mem_tag_set(grp, RVR_MALLOC_STATIC);


            for(int i = 0;i<32;i++)
            {
               if(RvR_key_down(RVR_KEY_SPACE))
                  RvR_render_present();

               uint32_t pos = grp->row_offsets[i];
               int count = grp->data[pos++];
               for(int j = 0;j<count;j++)
               {
                  int start = grp->data[pos++]+sx;
                  int len = grp->data[pos++];
                  if(sy+i<0||sy+i>=RvR_yres())
                  {
                     pos+=len;
                     continue;
                  }

                  int free_before = freed;
                  int alloc_before = alloc;

                  //Clip to screen
                  uint8_t *src = grp->data+pos;
                  Span *s = span_new();
                  s->x0 = start;
                  s->x1 = start+len;
                  if(s->x0<0)
                  {
                     src+=-s->x0;
                     s->x0 = 0;
                  }
                  if(s->x1>=RvR_xres())
                  {
                     s->x1 = RvR_xres();
                  }

                  //printf("%d\n",sy+i);

                  //if(spans[sy+i]==NULL)
                  //{
                     //span_draw(s->x0,s->x1,src,sy+i);
                     //spans[sy+i] = s;
                  //}
                  //else
                  //{
                  span_add(&spans[sy+i],s,src,sy+i);
                     if(sy+i==32)
                        printf("(%d %d) --> ",s->x0,s->x1);
                     if(sy+i==32)
                     {
                        Span *cur = spans[sy+i];
                        while(cur!=NULL)
                        {
                           printf("(%d %d) ",cur->x0,cur->x1);
                           cur = cur->next;
                        }
                        printf("; allocated: %d, freed: %d\n",alloc-alloc_before,freed-free_before);
                     }
                  //}

                  pos+=len;
                  /*int len = grp->data[pos++];
                  for(int l = 0;l<len;l++)
                  {
                     RvR_framebuffer()[(i+16)*RvR_xres()+start+l+16] = grp->data[pos++];
                  }*/
               }
               //if(grp->row_offsets[i]==UINT32_MAX)
                  //continue;
            }

            RvR_mem_tag_set(grp, RVR_MALLOC_CACHE);
         }
      }
   }
}

static Span *span_new()
{
   if(span_pool== NULL)
   {
      Span *s = RvR_malloc(sizeof(*s)*256,"iso spans");
      memset(s,0,sizeof(*s)*256);
      for(int i = 0;i<255;i++)
         s[i].next = &s[i+1];
      span_pool = s;
   }

   Span *s = span_pool;
   span_pool = s->next;
   memset(s,0,sizeof(*s));

   alloc++;

   return s;
}

static void span_free(Span *s)
{
   if(s==NULL)
      return;

   freed++;

   Span *n = s->next;

   s->next = span_pool;
   span_pool = s;

   span_free(n);
}

static void span_add(Span **tree, Span *s, uint8_t *src, int y)
{
   if(s->x0>=s->x1)
   {
      span_free(s);
      return;
   }

   if((*tree)==NULL)
   {
      span_draw(s->x0,s->x1,src,y);
      *tree = s;
      s->prev_next = tree;
      s->next = NULL;
      return;
   }

   Span *start = *tree;
   Span *end = *tree;

   while(start->next!=NULL&&s->x0>start->x1)
      start = start->next;
   while(end->next!=NULL&&s->x1>end->x1)
      end = end->next;

   //At end
   if(s->x0>start->x1)
   {
      span_draw(s->x0,s->x1,src,y);
      start->next = s;
      s->next = NULL;
      s->prev_next = &(start->next);
      return;
   }

   if(start==end)
   {
      if(s->x1<start->x0)
      {
         span_draw(s->x0,s->x1,src,y);
         Span **prev = start->prev_next;
         *(start->prev_next) = s;
         s->next = start;
         s->prev_next = prev;
         start->prev_next = &(s->next);
         return;
      }
   }

   //Merge s and start
   span_draw(s->x0,start->x0,src,y);
   start->x0 = RvR_min(s->x0,start->x0);

   //All between start and end --> merge into start; draw in gaps
   Span *cur = start->next;
   int x0 = start->x1;
   if(start!=end)
   {
      while(cur!=NULL&&cur!=end&&cur->next!=end)
      {
         Span *next = cur->next;
         start->x1 = cur->x1;
         start->next = next;
         span_draw(x0,cur->x0,src+(x0-s->x0),y);
         x0 = cur->x1;
         cur->next = NULL;
         span_free(cur);
         if(next!=NULL)
            next->prev_next = &(start->next);

         cur = next;
      }
   }

   if(s->x1<end->x0)
   {
      span_draw(x0,s->x1,src+(x0-s->x0),y);
      start->x1 = s->x1;
      span_free(s);
      return;
   }
   else if(s->x1<=end->x1)
   {
      span_draw(x0,end->x0,src+(x0-s->x0),y);
      start->x1 = end->x1;
      span_free(s);
      if(start!=end)
      {
         Span *next = end->next;
         end->next = NULL;
         span_free(end);
         start->next = next;
         if(next!=NULL)
            next->prev_next = &(start->next);
      }
      return;
   }
   else
   {
      span_draw(x0,end->x0,src+(x0-s->x0),y);
      x0 = end->x1;
      span_draw(x0,s->x1,src+(x0-s->x0),y);
      start->x1 = s->x1;
      span_free(s);

      if(start!=end)
      {
         Span *next = end->next;
         end->next = NULL;
         span_free(end);
         start->next = next;
         if(next!=NULL)
            next->prev_next = &(start->next);
      }
      return;
   }

}

static void span_draw(int x0, int x1, uint8_t *src, int y)
{
   if(x1-x0<=0)
      return;

   memcpy(RvR_framebuffer()+y*RvR_xres()+x0,src,x1-x0);
}
//-------------------------------------
