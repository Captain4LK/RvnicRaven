/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "world_gen.h"
#include "world.h"
#include "area.h"
#include "tile.h"
#include "region.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static int32_t rand_offset(RvR_rand_pcg *rand, int level, int32_t var);
//-------------------------------------

//Function implementations

Area *area_gen(World *w, uint32_t seed, uint16_t ax, uint16_t ay, uint8_t dimx, uint8_t dimy, uint8_t dimz, uint16_t id)
{
   RvR_rand_pcg rand = {0};
   RvR_rand_pcg_seed(&rand, seed);

   int stridey = dimy * 32 + 1;
   int stridex = dimx * 32 + 1;
   int32_t *elevation = RvR_malloc(sizeof(*elevation) * stridex * stridey, "AreaGen elevation");
   int32_t *temperature = RvR_malloc(sizeof(*temperature) * stridex * stridey, "AreaGen temperature");
   int32_t *rainfall = RvR_malloc(sizeof(*rainfall) * stridex * stridey, "AreaGen rainfall");
   for(int i = 0; i<stridex * stridey; i++)
   {
      elevation[i] = -1;
      temperature[i] = -1;
      rainfall[i] = -1;
   }
   int stride = dimy * 32 + 1;

   //Init grid
   for(int y = 0; y<dimy; y++)
   {
      for(int x = 0; x<dimx; x++)
      {
         elevation[y * 32 * stride + x * 32] = world_elevation(w, ax + x, ay + y);
         elevation[y * 32 * stride + (x + 1) * 32] = world_elevation(w, ax + x + 1, ay + y);
         elevation[(y + 1) * 32 * stride + x * 32] = world_elevation(w, ax + x, ay + y + 1);
         elevation[(y + 1) * 32 * stride + (x + 1) * 32] = world_elevation(w, ax + x + 1, ay + y + 1);
      }
   }

   //Diamond square
   for(int l = 0; l<6; l++)
   {
      int dimx_level = dimx << l;
      int dimy_level = dimy << l;
      int dimx_rest = 32 >> l;
      int dimy_rest = 32 >> l;

      //Diamond
      for(int y = 0; y<dimy_level; y++)
      {
         for(int x = 0; x<dimx_level; x++)
         {
            int32_t e0 = elevation[(y) * dimy_rest * stride + (x) * dimx_rest];
            int32_t e1 = elevation[(y) * dimy_rest * stride + (x + 1) * dimx_rest];
            int32_t e2 = elevation[(y + 1) * dimy_rest * stride + (x) * dimx_rest];
            int32_t e3 = elevation[(y + 1) * dimy_rest * stride + (x + 1) * dimx_rest];

            if(elevation[(y * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest + dimx_rest / 2]==-1)
               elevation[(y * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest + dimx_rest / 2] = (e0 + e1 + e2 + e3) / 4 + rand_offset(&rand, 5 + l, w->preset.var_elevation);
         }
      }

      //Square
      for(int y = 0; y<dimy_level; y++)
      {
         for(int x = 0; x<dimx_level; x++)
         {
            int32_t e0 = elevation[(y) * dimy_rest * stride + (x) * dimx_rest];
            int32_t e1 = elevation[(y) * dimy_rest * stride + (x + 1) * dimx_rest];
            int32_t e2 = elevation[(y + 1) * dimy_rest * stride + (x) * dimx_rest];
            int32_t e3 = elevation[(y + 1) * dimy_rest * stride + (x + 1) * dimx_rest];
            int32_t e4 = elevation[(y * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest + dimx_rest / 2];
            int32_t e5 = (e0 + e3 + e4) / 3;
            int32_t e6 = (e0 + e1 + e4) / 3;

            if(x>0)
            {
               e5 = elevation[(y * dimy_rest + dimy_rest / 2) * stride + (x - 1) * dimx_rest + dimx_rest / 2];
            }
            if(y>0)
            {
               e6 = elevation[((y - 1) * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest + dimx_rest / 2];
            }

            if(elevation[(y * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest]==-1)
               elevation[(y * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest] = (e0 + e3 + e5 + e4) / 4 + rand_offset(&rand, 5 + l, w->preset.var_elevation);

            if(elevation[(y * dimy_rest) * stride + x * dimx_rest + dimx_rest / 2]==-1)
               elevation[(y * dimy_rest) * stride + x * dimx_rest + dimx_rest / 2] = (e0 + e1 + e6 + e4) / 4 + rand_offset(&rand, 5 + l, w->preset.var_elevation);

            if(x==dimx_level - 1&&elevation[(y * dimy_rest + dimy_rest / 2) * stride + (x + 1) * dimx_rest]==-1)
               elevation[(y * dimy_rest + dimy_rest / 2) * stride + (x + 1) * dimx_rest] = (e1 + e2 + e4) / 3 + rand_offset(&rand, 5 + l, w->preset.var_elevation);

            if(y==dimy_level - 1&&elevation[((y + 1) * dimy_rest) * stride + x * dimx_rest + dimx_rest / 2]==-1)
               elevation[((y + 1) * dimy_rest) * stride + x * dimx_rest + dimx_rest / 2] = (e2 + e3 + e4) / 3 + rand_offset(&rand, 5 + l, w->preset.var_elevation);
         }
      }

#if 1
      //Recalulate squares
      for(int y = 0; y<dimy_level; y++)
      {
         for(int x = 0; x<dimx_level; x++)
         {
            if((x * dimx_rest) % (32)==0&&(y * dimy_rest) % (32)==0&&l!=5)
               continue;

            int32_t sum = 0;
            int32_t count = 0;
            if(x * dimx_rest - dimx_rest / 2>=0) {sum += elevation[(y) * dimy_rest * stride + (x) * dimx_rest - dimx_rest / 2]; count++;}
            if(x * dimx_rest + dimx_rest / 2<dimx * 32) {sum += elevation[(y) * dimy_rest * stride + (x) * dimx_rest + dimx_rest / 2]; count++;}
            if(y - 1>=0) {sum += elevation[((y) * dimy_rest - dimy_rest / 2) * stride + (x) * dimx_rest]; count++;}
            if(y + 1<dimy_level) {sum += elevation[((y) * dimy_rest + dimy_rest / 2) * stride + (x) * dimx_rest]; count++;}

            elevation[y * dimy_rest * stride + x * dimx_rest] = sum / count + rand_offset(&rand, 5 + l, w->preset.var_elevation);
         }
      }
#endif
   }

   int32_t max = INT32_MIN;
   int32_t min = INT32_MAX;
   for(int i = 0; i<stridex * stridey; i++)
   {
      if(elevation[i]>max)
      {
         max = elevation[i];
      }
      if(elevation[i]<min)
         min = elevation[i];
   }

   Area *a = area_create(w, ax, ay, dimx, dimy, dimz, id);
   for(int y = 0; y<dimy * 32; y++)
   {
      for(int x = 0; x<dimx * 32; x++)
      {
         int z = (elevation[y * stride + x] - min) / 1024 + 1;
         for(; z>0; z--)
         {
            area_set_tile(a, point(x, y, dimz * 32 - z), tile_set_discovered(tile_make_wall(2, 2), 0, 0));
         }
      }
   }

   RvR_free(elevation);
   RvR_free(temperature);
   RvR_free(rainfall);

   //TODO(Captain4LK): slopes
   for(int y = 0; y<dimy * 32; y++)
   {
      for(int x = 0; x<dimx * 32; x++)
      {
         for(int z = 0; z<dimz * 32; z++)
         {
            uint32_t t = area_tile(a, point(x, y, z));
            if(!tile_has_wall(t)&&tile_has_floor(area_tile(a, point(x, y, z + 1))))
            {
               uint8_t index = 0;
               if(tile_has_wall(area_tile(a, point(x - 1, y, z))))
                  index |= 1;
               if(tile_has_wall(area_tile(a, point(x, y - 1, z))))
                  index |= 2;
               if(tile_has_wall(area_tile(a, point(x + 1, y, z))))
                  index |= 4;
               if(tile_has_wall(area_tile(a, point(x, y + 1, z))))
                  index |= 8;
               if(tile_has_wall(area_tile(a, point(x - 1, y - 1, z))))
                  index |= 16;
               if(tile_has_wall(area_tile(a, point(x + 1, y - 1, z))))
                  index |= 32;
               if(tile_has_wall(area_tile(a, point(x + 1, y + 1, z))))
                  index |= 64;
               if(tile_has_wall(area_tile(a, point(x - 1, y + 1, z))))
                  index |= 128;

               if(index!=0)
               {
                  uint8_t slope_var[256] =
                  {
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     9, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     8, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     11, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     8, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     10, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     11, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                     0, 2, 1, 5, 0, 0, 4, 0, 3, 6, 0, 0, 7, 0, 1, 0,
                  };

                  area_set_tile(a, point(x, y, z), tile_set_discovered(tile_make_slope(2, slope_var[index]), 0, 0));
               }
            }
         }
      }
   }

   return a;
}

static int32_t rand_offset(RvR_rand_pcg *rand, int level, int32_t var)
{
   return RvR_rand_pcg_next_range(rand, -var, var) / (1 << level);
}
//-------------------------------------
