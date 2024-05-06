/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
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
static void span_draw(Span *s, uint8_t *src, int y);
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
   srand(0);
   for(int z = 0;z<32;z++)
   {
      for(int y = 32;y>=0;y--)
      {
         for(int x = 0;x<32;x++)
         {
            if(rand()&1)
               continue;
            int sx = x*16+y*16;
            int sy = z*12-8*x+8*y;
            GRP_block *grp = grp_block_get(0);
            RvR_mem_tag_set(grp, RVR_MALLOC_STATIC);


            for(int i = 0;i<32;i++)
            {
               //if(RvR_key_down(RVR_KEY_SPACE))
                  //RvR_render_present();

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

                  if(spans[sy+i]==NULL)
                  {
                     span_draw(s,src,sy+i);
                     spans[sy+i] = s;
                  }
                  else
                  {
                     span_add(&spans[sy+i],s,src,sy+i);
                  }

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

#if 1

static void span_free(Span *s)
{
   if(s==NULL)
      return;

   //Search rightmost
   freed++;

   Span *n = s->next;

   s->next = span_pool;
   span_pool = s;

   span_free(n);
   //span_free(r);
}

static void span_add(Span **tree, Span *s, uint8_t *src, int y)
{
   if((*tree)==NULL)
   {
      span_draw(s,src,y);
      *tree = s;
      s->prev_next = tree;
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
      span_draw(s,src,y);
      start->next = s;
      s->next = NULL;
      s->prev_next = &(start->next);
      return;
   }

   if(start==end)
   {
      if(s->x1<start->x0)
      {
         span_draw(s,src,y);
         return;
      }
   }

   //Merge s and start

   /*if(s->x0<=s->x1)
   {
      span_free(s);
      return;
   }

   Span *left = tree;
   Span *right = tree;

   if(s->x0>=tree->x0)
   {
      while(left->right!=NULL&&s->x0>=left->x1)
         left = left->right;
      
      while(right->right!=NULL&&s->x1>=right->x1)
         right = right->right;
   }
   else if(s->x1<=tree->x1)
   {
      while(right->left!=NULL&&s->x1<right->x0)
         right = right->left;

      while(left->left!=NULL&&s->x0<left->x0)
         left = left->left;
   }

   if(s->x1<left->x0)
   {
      span_draw(s,src,y);
      Span *l = left->left;
      left->left = s;
      s->left = l;

      return;
   }
   else if(s->x0>left->x1)
   {
      span_draw(s,src,y);

      return;
   }*/

   /*if(s->x0<tree->x0)
   {
      while(left->left!=NULL&&s->x0<left->x0)
         left = left->left;
   }
   else if(s->x0>tree->x1)
   {
   }*/

   /*while(s->x0<s->x1)
   {
      if(s->x1<=tree->x0)
      {
         if(tree->left==NULL)
         {
            span_draw(s,src,y);

            if(s->x1==tree->x0)
            {
               tree->x0 = s->x0;
               span_free(s);
            }
            else
            {
               tree->left = s;
            }

            return;
         }

         tree = tree->left;
      }
      else if(s->x0>=tree->x1)
      {
         if(tree->right==NULL)
         {
            span_draw(s,src,y);

            if(s->x0==tree->x1)
            {
               tree->x1 = s->x1;
               span_free(s);
            }
            else
            {
               tree->right = s;
            }

            return;
         }
         tree = tree->right;
      }
      else if(s->x0>=tree->x0)
      {
         if(s->x1<=tree->x1)
         {
            span_free(s);
            return;
         }

         if(s->x0<=tree->x1)
         {
            int diff = tree->x1-s->x0;
            src+=diff;
            s->x0 = tree->x1;

            if(tree->right==NULL)
            {
               span_draw(s,src,y);
               tree->x1 = s->x1;
               span_free(s);
               return;
            }

            tree = tree->right;
         }
      }
      else if(s->x1>tree->x1)
      {
         Span *s0 = span_new();
         *s0 = *s;
         int diff = tree->x1-s0->x0;
         s0->x0 = tree->x1;
         if(tree->right==NULL)
         {
            span_draw(s0,src+diff,y);
            tree->x1 = s->x1;
            span_free(s0);
         }
         else
         {
            span_add(tree->right,s0,src+diff,y);
         }

         if(s->x0>=tree->x0)
         {
            span_free(s);
            return;
         }
         
         s->x1 = tree->x0;

         if(tree->left==NULL)
         {
            span_draw(s,src,y);
            tree->x0 = s->x0;
            span_free(s);
            return;
         }

         tree = tree->left;
      }
      else
      {
         if(s->x0>=tree->x0)
         {
            span_free(s);
            return;
         }

         s->x1 = tree->x0;

         if(tree->left==NULL)
         {
            span_draw(s,src,y);
            tree->x0 = s->x0;
            span_free(s);
            return;
         }

         tree = tree->left;
      }
   }

   span_free(s);*/
}

#else

static void span_free(Span *s)
{
   if(s==NULL)
      return;

   freed++;

   Span *l = s->left;
   Span *r = s->right;

   s->left = span_pool;
   span_pool = s;

   span_free(l);
   span_free(r);
}

static void span_add(Span *tree, Span *s, uint8_t *src, int y)
{
   while(s->x0<s->x1)
   {
      if(s->x1<=tree->x0)
      {
         if(tree->left==NULL)
         {
            span_draw(s,src,y);

            if(s->x1==tree->x0)
            {
               tree->x0 = s->x0;
               span_free(s);
            }
            else
            {
               tree->left = s;
            }

            return;
         }

         tree = tree->left;
      }
      else if(s->x0>=tree->x1)
      {
         if(tree->right==NULL)
         {
            span_draw(s,src,y);

            if(s->x0==tree->x1)
            {
               tree->x1 = s->x1;
               span_free(s);
            }
            else
            {
               tree->right = s;
            }

            return;
         }
         tree = tree->right;
      }
      else if(s->x0>=tree->x0)
      {
         if(s->x1<=tree->x1)
         {
            span_free(s);
            return;
         }

         if(s->x0<=tree->x1)
         {
            int diff = tree->x1-s->x0;
            src+=diff;
            s->x0 = tree->x1;

            if(tree->right==NULL)
            {
               span_draw(s,src,y);
               tree->x1 = s->x1;
               span_free(s);
               return;
            }

            tree = tree->right;
         }
      }
      else if(s->x1>tree->x1)
      {
         Span *s0 = span_new();
         *s0 = *s;
         int diff = tree->x1-s0->x0;
         s0->x0 = tree->x1;
         if(tree->right==NULL)
         {
            span_draw(s0,src+diff,y);
            tree->x1 = s->x1;
            span_free(s0);
         }
         else
         {
            span_add(tree->right,s0,src+diff,y);
         }

         if(s->x0>=tree->x0)
         {
            span_free(s);
            return;
         }
         
         s->x1 = tree->x0;

         if(tree->left==NULL)
         {
            span_draw(s,src,y);
            tree->x0 = s->x0;
            span_free(s);
            return;
         }

         tree = tree->left;
      }
      else
      {
         if(s->x0>=tree->x0)
         {
            span_free(s);
            return;
         }

         s->x1 = tree->x0;

         if(tree->left==NULL)
         {
            span_draw(s,src,y);
            tree->x0 = s->x0;
            span_free(s);
            return;
         }

         tree = tree->left;
      }
   }

   span_free(s);
}

#endif

static void span_draw(Span *s, uint8_t *src, int y)
{
   if(s->x1-s->x0<=0)
      return;

   memcpy(RvR_framebuffer()+y*RvR_xres()+s->x0,src,s->x1-s->x0);
}
//-------------------------------------
