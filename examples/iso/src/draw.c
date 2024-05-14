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

static int row_done[RVR_YRES_MAX];

static Map *iso_map;
static Camera *iso_cam;
//-------------------------------------

//Function prototypes
static Span *span_new();
static void span_free(Span *s);
static void span_add(Span **tree, int x0, int x1, uint8_t *src, int y);
static void span_draw(int x0, int x1, uint8_t *src, int y);
//-------------------------------------

//Function implementations

void draw_begin(Map *map, Camera *camera)
{
   RvR_render_clear(0);
   for(int i = 0;i<RvR_yres();i++)
   {
      span_free(spans[i]);
      spans[i] = NULL;
      row_done[i] = 0;
   }

   printf("Allocated: %d; Freed: %d\n",alloc,freed);
   alloc = 0;
   freed = 0;

   iso_map = map;
   iso_cam = camera;
}

void draw_end()
{
}

void draw_map()
{
   int cx = -1+(-2*RvR_yres()-2*iso_cam->y+iso_cam->x)/32;
   int cy = (RvR_yres()+iso_cam->y)/8+cx;

   for(int i = 0;i<4;i++)
   {
      int tx = cx;
      int ty = cy;

      cy--;
      cx--;
   }
#if 0
   srand(1);
   for(int z = 0;z<32;z++)
   {
      for(int y = 32;y>=0;y--)
      {
         for(int x = 0;x<32;x++)
         {
            //if(rand()&1)
               //continue;
            int sx = x*16+y*16-128;
            //int sy = -8*x+8*y;
            int sy = z*12-8*x+8*y+64;
            GRP_block *grp = grp_block_get(0);

            if(sx>=RvR_xres()||sx+32<0||sy>=RvR_yres()||sy+32<0)
               continue;

            RvR_mem_tag_set(grp, RVR_MALLOC_STATIC);

            for(int i = 0;i<32;i++)
            {
               //if(RvR_key_down(RVR_KEY_SPACE))
                  //RvR_render_present();

               if(sy+i<0||sy+i>=RvR_yres()||row_done[sy+i])
                  continue;

               uint32_t pos = grp->row_offsets[i];
               int count = grp->data[pos++];
               for(int j = 0;j<count;j++)
               {
                  int start = grp->data[pos++]+sx;
                  int len = grp->data[pos++];

                  //Clip to screen
                  uint8_t *src = grp->data+pos;
                  int x0 = start;
                  int x1 = start+len;
                  if(x0<0)
                  {
                     src+=-x0;
                     x0 = 0;
                  }
                  if(x1>=RvR_xres())
                  {
                     x1 = RvR_xres();
                  }

                  span_add(&spans[sy+i],x0,x1,src,sy+i);
                  if(spans[sy+i]!=NULL&&spans[sy+i]->x0==0&&spans[sy+i]->x1==RvR_xres())
                     row_done[sy+i] = 1;

                  pos+=len;
               }
            }

            RvR_mem_tag_set(grp, RVR_MALLOC_CACHE);
         }
      }
   }
#endif
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

static void span_add(Span **tree, int x0, int x1, uint8_t *src, int y)
{
   if(x0>=x1)
      return;

   if((*tree)==NULL)
   {
      span_draw(x0,x1,src,y);
      Span *s = span_new();
      s->x0 = x0;
      s->x1 = x1;
      *tree = s;
      s->prev_next = tree;
      s->next = NULL;
      return;
   }

   if(x0>=(*tree)->x0&&x1<=(*tree)->x1)
      return;

   Span *start = *tree;
   Span *end = *tree;

   while(start->next!=NULL&&x0>start->x1)
      start = start->next;
   while(end->next!=NULL&&x1>end->x1)
      end = end->next;

   //At end
   if(x0>start->x1)
   {
      span_draw(x0,x1,src,y);
      Span *s = span_new();
      s->x0 = x0;
      s->x1 = x1;
      start->next = s;
      s->next = NULL;
      s->prev_next = &(start->next);
      return;
   }

   if(start==end)
   {
      if(x1<start->x0)
      {
         span_draw(x0,x1,src,y);
         Span **prev = start->prev_next;
         Span *s = span_new();
         s->x0 = x0;
         s->x1 = x1;
         *(start->prev_next) = s;
         s->next = start;
         s->prev_next = prev;
         start->prev_next = &(s->next);
         return;
      }
   }

   //Merge s and start
   span_draw(x0,start->x0,src,y);
   start->x0 = RvR_min(x0,start->x0);

   //All between start and end --> merge into start; draw in gaps
   Span *cur = start->next;
   int cx0 = start->x1;
   if(start!=end)
   {
      while(cur!=NULL&&cur!=end)
      {
         Span *next = cur->next;
         start->x1 = cur->x1;
         start->next = next;
         span_draw(cx0,cur->x0,src+(cx0-x0),y);
         cx0 = cur->x1;
         cur->next = NULL;
         span_free(cur);
         if(next!=NULL)
            next->prev_next = &(start->next);

         cur = next;
      }
   }

   if(x1<end->x0)
   {
      span_draw(cx0,x1,src+(cx0-x0),y);
      start->x1 = x1;
      return;
   }
   else if(x1<=end->x1)
   {
      span_draw(cx0,end->x0,src+(cx0-x0),y);
      start->x1 = end->x1;
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
      span_draw(cx0,end->x0,src+(cx0-x0),y);
      cx0 = end->x1;
      span_draw(cx0,x1,src+(cx0-x0),y);
      start->x1 = x1;

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
