/*
RvnicRaven - palette and color luts

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR/RvR_rw.h"
#include "RvR/RvR_malloc.h"
#include "RvR/RvR_math.h"
#include "RvR/RvR_pak.h"
#include "RvR/RvR_palette.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t *rvr_pal_shade_table = NULL;
static uint8_t rvr_pal_trans_table[256][256] = {0};
static RvR_color *rvr_palette = NULL;
//-------------------------------------

//Function prototypes
static void rvr_pal_calculate_colormap();
static uint8_t rvr_pal_find_closest(int r, int g, int b);
//-------------------------------------

//Function implementations

void RvR_palette_load(uint16_t id)
{
   unsigned size_in = 0;
   uint8_t *mem_pak = NULL;
   RvR_rw rw = {0};

   //Allocate palette if it isn't yet
   if(rvr_palette==NULL)
      rvr_palette = RvR_malloc(sizeof(*rvr_palette) * 256, "RvR palette");

   //Format lump name
   //Palettes must be named in this exact way (e.g. PAL00000)
   char tmp[64];
   snprintf(tmp, 64, "PAL%05d", id);

   //Read palette from lump and create rw stream
   mem_pak = RvR_lump_get(tmp, &size_in);
   RvR_rw_init_mem(&rw, mem_pak, size_in, size_in);

   //Read palette and perform post processing
   for(unsigned i = 0; i<256; i++)
   {
      rvr_palette[i].r = RvR_rw_read_u8(&rw);
      rvr_palette[i].g = RvR_rw_read_u8(&rw);
      rvr_palette[i].b = RvR_rw_read_u8(&rw);
      rvr_palette[i].a = 255;
   }
   rvr_pal_calculate_colormap();

   //Cleanup
   RvR_rw_close(&rw);
   RvR_free(mem_pak);

   return;
}

static void rvr_pal_calculate_colormap()
{
   if(rvr_pal_shade_table==NULL)
      rvr_pal_shade_table = RvR_malloc(sizeof(*rvr_pal_shade_table) * 256 * 64, "RvR palette shade table");

   //Distance fading
   for(int x = 0; x<256; x++)
   {
      for(int y = 0; y<64; y++)
      {
         if(x==0)
         {
            rvr_pal_shade_table[y * 256 + x] = 0;
            continue;
         }

         int r = RvR_max(0, RvR_min(255, ((int)rvr_palette[x].r * (63 - y)) / 63));
         int g = RvR_max(0, RvR_min(255, ((int)rvr_palette[x].g * (63 - y)) / 63));
         int b = RvR_max(0, RvR_min(255, ((int)rvr_palette[x].b * (63 - y)) / 63));

         rvr_pal_shade_table[y * 256 + x] = rvr_pal_find_closest(r, g, b);
      }
   }

   //Transparancy
   for(int i = 0; i<256; i++)
   {
      for(int j = 0; j<256; j++)
      {
         //Special case: entry 0 is transparent
         if(i==0)
         {
            rvr_pal_trans_table[i][j] = (uint8_t)j;
            continue;
         }
         else if(j==0)
         {
            rvr_pal_trans_table[i][j] = (uint8_t)i;
            continue;
         }

         int32_t t = 2048 / 3;
         uint8_t r = (uint8_t)RvR_max(0, RvR_min(255, (t * rvr_palette[i].r + (1024 - t) * rvr_palette[j].r) / 1024));
         uint8_t g = (uint8_t)RvR_max(0, RvR_min(255, (t * rvr_palette[i].g + (1024 - t) * rvr_palette[j].g) / 1024));
         uint8_t b = (uint8_t)RvR_max(0, RvR_min(255, (t * rvr_palette[i].b + (1024 - t) * rvr_palette[j].b) / 1024));

         rvr_pal_trans_table[i][j] = rvr_pal_find_closest(r, g, b);
      }
   }
}

static uint8_t rvr_pal_find_closest(int r, int g, int b)
{
   int32_t best_dist = INT32_MAX;
   uint8_t best_index = 0;

   for(int i = 0; i<256; i++)
   {
      int32_t dist = 0;
      dist += (r - rvr_palette[i].r) * (r - rvr_palette[i].r);
      dist += (g - rvr_palette[i].g) * (g - rvr_palette[i].g);
      dist += (b - rvr_palette[i].b) * (b - rvr_palette[i].b);

      if(dist<best_dist)
      {
         best_index = (uint8_t)i;
         best_dist = dist;
      }
   }

   return best_index;
}

RvR_color *RvR_palette()
{
   return rvr_palette;
}

uint8_t *RvR_shade_table(uint8_t light)
{
   return &rvr_pal_shade_table[light * 256];
}

uint8_t RvR_blend(uint8_t c0, uint8_t c1)
{
   return rvr_pal_trans_table[c0][c1];
}
//-------------------------------------
