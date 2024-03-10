/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "sprite.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Sprite sprites[SPRITE_CACHE_COUNT];
uint16_t sprites_next;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void sprites_init()
{
   sprites_next = 0;
   for(int i = 0; i<SPRITE_CACHE_COUNT; i++)
   {
      RvR_texture_create(65536 - SPRITE_CACHE_COUNT + i, 32, 36);
      sprites[i].owner = NULL;
   }
}

int sprite_valid(uint16_t id, void *owner)
{
   if(owner==NULL||id>=SPRITE_CACHE_COUNT)
      return 0;

   return sprites[id].owner==owner;
}

uint16_t sprite_new(void *owner)
{
   sprites[sprites_next].owner = owner;
   uint16_t id = sprites_next;
   sprites_next = (sprites_next + 1) % SPRITE_CACHE_COUNT;
   return id;
}

uint16_t sprite_texture(uint16_t id)
{
   return 65536 - SPRITE_CACHE_COUNT + id;
}

void sprite_clear(uint16_t id, uint8_t color)
{
   RvR_texture *tex = RvR_texture_get(65536 - SPRITE_CACHE_COUNT + id);
   memset(tex->data, color, tex->width * tex->height);
}

void sprite_draw_sprite(uint16_t id, uint16_t tex, int x, int y, int sx, int sy, int width, int height)
{
   RvR_texture *dst = RvR_texture_get(65536 - SPRITE_CACHE_COUNT + id);
   RvR_texture *src = RvR_texture_get(tex);

   int start_x = 0;
   int start_y = 0;
   int end_x = width;
   int end_y = height;
   if(x<0)
      start_x = -x;
   if(y<0)
      start_y = -y;
   if(x + end_x>dst->width)
      end_x = width + (dst->width - x - end_x);
   if(y + end_y>dst->height)
      end_y = height + (dst->height - y - end_y);

   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t *psrc = &src->data[start_x + sx + (start_y + sy) * src->width];
   uint8_t *pdst = &dst->data[x + y * dst->width];
   int src_step = -(end_x - start_x) + src->width;
   int dst_step = dst->width - (end_x - start_x);

   for(int dy = start_y; dy<end_y; dy++, pdst += dst_step, psrc += src_step)
   {
      for(int dx = start_x; dx<end_x; dx++, psrc++, pdst++)
      {
         *pdst = (*psrc!=255)?*psrc:*pdst;
      }
   }

   /*int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = width;
   int draw_end_y = height;
   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x+draw_end_x>target_pal->width)
      draw_end_x = width+(target_pal->width-x-draw_end_x);
   if(y+draw_end_y>target_pal->height)
      draw_end_y = height+(target_pal->height-y-draw_end_y);

   //Clip dst sprite
   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t *src = &s->data[draw_start_x+ox+(draw_start_y+oy)*s->width];
   uint8_t *dst = &target_pal->data[x+y*target_pal->width];
   int src_step = -(draw_end_x-draw_start_x)+s->width;
   int dst_step = target_pal->width-(draw_end_x-draw_start_x);

   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step,src+=src_step)
      for(int x1 = draw_start_x;x1<draw_end_x;x1++,src++,dst++)
         *dst = *src?*src:*dst;*/
}
//-------------------------------------
