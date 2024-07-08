/*
RvnicRaven - iso roguelike

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "config.h"
#include "world_gen.h"
#include "world.h"
#include "area.h"
#include "tile.h"
#include "region.h"
#include "chunk.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static Chunk *chunk_gen(World *w, int cx, int cy, int cz);
static int32_t rand_offset(RvR_rand_pcg *rand, int level, int32_t var);
//-------------------------------------

//Function implementations

Chunk *chunk_get(World *w, int x, int y, int z)
{
   //TODO(Captian4LK): check if chunk in existing area

   return chunk_gen(w, x, y, z);
}

Point chunk_pos_to_area(const Area *a, const Chunk *c, Point pos)
{
   if(a==NULL||c==NULL)
      return pos;

   return point((c->x-a->cx+AREA_DIM/2)*32+pos.x,
                (c->y-a->cy+AREA_DIM/2)*32+pos.y,
                (c->z-a->cz+AREA_DIM/2)*32+pos.z);
}

static Chunk *chunk_gen(World *w, int cx, int cy, int cz)
{
   //TODO(Captain4LK): empty if out of bounds

   Chunk *c = RvR_malloc(sizeof(*c), "Chunk");
   c->x = cx;
   c->y = cy;
   c->z = cz;
   c->items = NULL;
   c->entities = NULL;
   memset(c->item_grid, 0, sizeof(c->item_grid));
   memset(c->entity_grid, 0, sizeof(c->entity_grid));

   for(int z = 0;z<32;z++)
   {
      for(int y = 0;y<32;y++)
      {
         for(int x = 0;x<32;x++)
         {
            int wx = cx*32+x;
            int wy = cy*32+y;
            int wz = cz*32+z;

            if(wz>=1024)
               c->tiles[z*32*32+y*32+x] = tile_set_discovered(tile_make_wall(2,2),0,0);
            else
               c->tiles[z*32*32+y*32+x] = tile_set_discovered(tile_make_empty(),0,0);
         }
      }
   }

   return c;
#if 0
   Chunk *c = RvR_malloc(sizeof(*c), "Chunk");
   c->x = cx;
   c->y = cy;
   c->z = cz;

   uint32_t seed = 0;

   int stridey = 3 * 32 + 1;
   int stridex = 3 * 32 + 1;
   int32_t *elevation = RvR_malloc(sizeof(*elevation) * stridex * stridey, "AreaGen elevation");
   int32_t *temperature = RvR_malloc(sizeof(*temperature) * stridex * stridey, "AreaGen temperature");
   int32_t *rainfall = RvR_malloc(sizeof(*rainfall) * stridex * stridey, "AreaGen rainfall");
   RvR_rand_pcg *pcg = RvR_malloc(sizeof(*pcg) * stridex * stridey, "AreaGen pcg");

   for(int i = 0; i<stridex * stridey; i++)
   {
      elevation[i] = -1;
      temperature[i] = -1;
      rainfall[i] = -1;
   }

   //Init rngs
   for(int y = 0; y<stridey; y++)
   {
      for(int x = 0; x<stridex; x++)
      {
         uint32_t buf[3] = {seed, cx + x, cy + y};
         RvR_rand_pcg_seed(&pcg[y * stridex + y], RvR_fnv32a_buf(buf, sizeof(buf), RVR_HASH_FNV32_INIT));
      }
   }

   //Init grid
   int stride = 3 * 32 + 1;
   for(int y = 0; y<3; y++)
   {
      for(int x = 0; x<3; x++)
      {
         elevation[y * 32 * stride + x * 32] = world_elevation(w, cx + x, cy + y);
         elevation[y * 32 * stride + (x + 1) * 32] = world_elevation(w, cx + x + 1, cy + y);
         elevation[(y + 1) * 32 * stride + x * 32] = world_elevation(w, cx + x, cy + y + 1);
         elevation[(y + 1) * 32 * stride + (x + 1) * 32] = world_elevation(w, cx + x + 1, cy + y + 1);
      }
   }

   //Diamond square
   for(int l = 0; l<6; l++)
   {
      int dimx_level = 3 << l;
      int dimy_level = 3 << l;
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
            size_t index = (y * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest + dimx_rest / 2;

            if(elevation[index]==-1)
               elevation[index] = (e0 + e1 + e2 + e3) / 4 + rand_offset(&pcg[index], 5 + l, w->preset.var_elevation);
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

            size_t index = (y * dimy_rest + dimy_rest / 2) * stride + x * dimx_rest;
            if(elevation[index]==-1)
               elevation[index] = (e0 + e3 + e5 + e4) / 4 + rand_offset(&pcg[index], 5 + l, w->preset.var_elevation);

            index = (y * dimy_rest) * stride + x * dimx_rest + dimx_rest / 2;
            if(elevation[index]==-1)
               elevation[index] = (e0 + e1 + e6 + e4) / 4 + rand_offset(&pcg[index], 5 + l, w->preset.var_elevation);

            index = (y * dimy_rest + dimy_rest / 2) * stride + (x + 1) * dimx_rest;
            if(x==dimx_level - 1&&elevation[index]==-1)
               elevation[index] = (e1 + e2 + e4) / 3 + rand_offset(&pcg[index], 5 + l, w->preset.var_elevation);

            index = ((y + 1) * dimy_rest) * stride + x * dimx_rest + dimx_rest / 2;
            if(y==dimy_level - 1&&elevation[index]==-1)
               elevation[index] = (e2 + e3 + e4) / 3 + rand_offset(&pcg[index], 5 + l, w->preset.var_elevation);
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
            if(x * dimx_rest + dimx_rest / 2<3 * 32) {sum += elevation[(y) * dimy_rest * stride + (x) * dimx_rest + dimx_rest / 2]; count++;}
            if(y - 1>=0) {sum += elevation[((y) * dimy_rest - dimy_rest / 2) * stride + (x) * dimx_rest]; count++;}
            if(y + 1<dimy_level) {sum += elevation[((y) * dimy_rest + dimy_rest / 2) * stride + (x) * dimx_rest]; count++;}

            size_t index = y * dimy_rest * stride + x * dimx_rest;
            elevation[index] = sum / count + rand_offset(&pcg[index], 5 + l, w->preset.var_elevation);
         }
      }
#endif
   }

   for(int z = 0;z<32;z++)
   {
      for(int y = 0;y<32;y++)
      {
         for(int x = 0;x<32;x++)
         {
         }
      }
   }

   RvR_free(elevation);
   RvR_free(temperature);
   RvR_free(rainfall);
   RvR_free(pcg);

   return c;

#endif
}

static int32_t rand_offset(RvR_rand_pcg *rand, int level, int32_t var)
{
   return RvR_rand_pcg_next_range(rand, -var, var) / (1 << level);
}
//-------------------------------------
