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
static void rand_pos_init(int32_t **pos, World *w);
static void rand_pos_free(int32_t **pos);
static void rand_pos(RvR_rand_pcg *rand, int32_t **pos, int32_t *x, int32_t *y);
//-------------------------------------

//Function implementations

void world_gen(World *w, uint32_t seed)
{
   RvR_rand_pcg rand = {0};
   RvR_rand_pcg_seed(&rand, seed);

   int dim = world_size_to_dim(w->size);
   int scale_log = RvR_log2(dim) - 4;
   int scale = 1 << scale_log;
   int stride = dim * 32 + 1;

   uint16_t *temperature = RvR_malloc(sizeof(*temperature) * stride * stride, "Map Gen temperature");
   uint16_t *wetness = RvR_malloc(sizeof(*wetness) * stride * stride, "Map Gen wetness");
   uint16_t *sky_top = RvR_malloc(sizeof(*sky_top) * stride * stride, "Map Gen sky top");
   uint16_t *srf_top = RvR_malloc(sizeof(*srf_top) * stride * stride, "Map Gen surface top");
   uint16_t *cv0_top = RvR_malloc(sizeof(*cv0_top) * stride * stride, "Map Gen cavern0 top");
   uint16_t *cv0_bot = RvR_malloc(sizeof(*cv0_bot) * stride * stride, "Map Gen cavern0 bot");
   uint16_t *cv1_top = RvR_malloc(sizeof(*cv1_top) * stride * stride, "Map Gen cavern1 top");
   uint16_t *cv1_bot = RvR_malloc(sizeof(*cv1_bot) * stride * stride, "Map Gen cavern1 bot");
   uint16_t *cv2_top = RvR_malloc(sizeof(*cv2_top) * stride * stride, "Map Gen cavern2 top");
   uint16_t *cv2_bot = RvR_malloc(sizeof(*cv2_bot) * stride * stride, "Map Gen cavern2 bot");

   //int32_t *elevation = RvR_malloc(sizeof(*elevation) * stride * stride, "Map Gen elevation");
   //int32_t *temperature = RvR_malloc(sizeof(*temperature) * stride * stride, "Map Gen temperature");
   //int32_t *rainfall = RvR_malloc(sizeof(*rainfall) * stride * stride, "Map Gen rainfall");

   for(int i = 0; i<stride * stride; i++)
   {
      temperature[i] = UINT16_MAX;
      wetness[i] = UINT16_MAX;
      sky_top[i] = UINT16_MAX;
      srf_top[i] = UINT16_MAX;
      cv0_top[i] = UINT16_MAX;
      cv0_bot[i] = UINT16_MAX;
      cv1_top[i] = UINT16_MAX;
      cv1_bot[i] = UINT16_MAX;
      cv2_top[i] = UINT16_MAX;
      cv2_bot[i] = UINT16_MAX;
      //elevation[i] = -1;
      //temperature[i] = -1;
      //rainfall[i] = -1;
   }

   //Basic parameters
   //-------------------------------------

   //Init with random elevation above sea level
   for(int y = 0; y<=16; y++)
   {
      for(int x = 0; x<=16; x++)
      {
         temperature[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 4 * 256, 80 * 256);
         wetness[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 4 * 256, 128 * 256);

         srf_top[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         sky_top[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         cv0_top[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         cv0_bot[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         cv1_top[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         cv1_bot[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         cv2_top[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         cv2_bot[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 256, 196 * 256);
         /*
         //Sea level at 128*1024
         elevation[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 80 * 1024, 196 * 1024);

         temperature[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 4 * 1024, 80 * 1024);
         rainfall[y * 32 * stride * scale + x * 32 * scale] = RvR_rand_pcg_next_range(&rand, 4 * 1024, 128 * 1024);
         */
      }
   }

   //Weigh with circle
   for(int y = 0; y<=dim; y++)
   {
      for(int x = 0; x<=dim; x++)
      {
         int32_t dist = (y - dim / 2) * (y - dim / 2) + (x - dim / 2) * (x - dim / 2);
         if(dist>(dim / 2) * (dim / 2))
         {
            srf_top[y * 32 * stride + x * 32] = 64 * 256;
            sky_top[y * 32 * stride + x * 32] = 64 * 256;
            cv0_top[y * 32 * stride + x * 32] = 64 * 256;
            cv0_bot[y * 32 * stride + x * 32] = 64 * 256;
            cv1_top[y * 32 * stride + x * 32] = 64 * 256;
            cv1_bot[y * 32 * stride + x * 32] = 64 * 256;
            cv2_top[y * 32 * stride + x * 32] = 64 * 256;
            cv2_bot[y * 32 * stride + x * 32] = 64 * 256;
         }
      }
   }

   //Edges are below sea level
   for(int i = 0; i<stride; i++)
   {
      srf_top[i] = 64 * 256;
      srf_top[(stride - 1) * stride + i] = 64 * 256;
      srf_top[i * stride] = 64 * 256;
      srf_top[i * stride + stride - 1] = 64 * 256;

      sky_top[i] = 64 * 256;
      sky_top[(stride - 1) * stride + i] = 64 * 256;
      sky_top[i * stride] = 64 * 256;
      sky_top[i * stride + stride - 1] = 64 * 256;

      cv0_top[i] = 64 * 256;
      cv0_top[(stride - 1) * stride + i] = 64 * 256;
      cv0_top[i * stride] = 64 * 256;
      cv0_top[i * stride + stride - 1] = 64 * 256;

      cv0_bot[i] = 64 * 256;
      cv0_bot[(stride - 1) * stride + i] = 64 * 256;
      cv0_bot[i * stride] = 64 * 256;
      cv0_bot[i * stride + stride - 1] = 64 * 256;

      cv1_top[i] = 64 * 256;
      cv1_top[(stride - 1) * stride + i] = 64 * 256;
      cv1_top[i * stride] = 64 * 256;
      cv1_top[i * stride + stride - 1] = 64 * 256;

      cv1_bot[i] = 64 * 256;
      cv1_bot[(stride - 1) * stride + i] = 64 * 256;
      cv1_bot[i * stride] = 64 * 256;
      cv1_bot[i * stride + stride - 1] = 64 * 256;

      cv2_top[i] = 64 * 256;
      cv2_top[(stride - 1) * stride + i] = 64 * 256;
      cv2_top[i * stride] = 64 * 256;
      cv2_top[i * stride + stride - 1] = 64 * 256;

      cv2_bot[i] = 64 * 256;
      cv2_bot[(stride - 1) * stride + i] = 64 * 256;
      cv2_bot[i * stride] = 64 * 256;
      cv2_bot[i * stride + stride - 1] = 64 * 256;
      /*
      elevation[i] = 64 * 1024;
      elevation[(stride - 1) * stride + i] = 64 * 1024;
      elevation[i * stride] = 64 * 1024;
      elevation[i * stride + stride - 1] = 64 * 1024;
      */
   }

   //North pole
   for(int i = 0; i<stride; i++)
   {
      temperature[i] = 8 * 256;
      temperature[32 * stride + i] = 8 * 256;
   }

   int32_t *pos = NULL;
   rand_pos_init(&pos, w);

   //Place up to n mountainpeaks
   for(int i = 0; i<w->preset.mountains_high; i++)
   {
      int x, y;
      rand_pos(&rand, &pos, &x, &y);
      srf_top[y * 32 * stride + x * 32] = 224 * 256 + RvR_rand_pcg_next_range(&rand, -w->preset.var_elevation, w->preset.var_elevation);
      //elevation[y * 32 * stride + x * 32] = 224 * 1024 + RvR_rand_pcg_next_range(&rand, -w->preset.var_elevation, w->preset.var_elevation);
   }

   //Place n deep lakes
   for(int i = 0; i<w->preset.lakes_deep; i++)
   {
      int x, y;
      rand_pos(&rand, &pos, &x, &y);
      srf_top[y * 32 * stride + x * 32] = 64 * 256 + RvR_rand_pcg_next_range(&rand, -w->preset.var_elevation, w->preset.var_elevation);
      //elevation[y * 32 * stride + x * 32] = 64 * 1024 + RvR_rand_pcg_next_range(&rand, -w->preset.var_elevation, w->preset.var_elevation);
   }

   //Place n shallow lakes
   for(int i = 0; i<w->preset.lakes_shallow; i++)
   {
      int x = RvR_rand_pcg_next_range(&rand, 0, dim);
      int y = RvR_rand_pcg_next_range(&rand, 0, dim);
      srf_top[y * 32 * stride + x * 32] = 124 * 256;
      //elevation[y * 32 * stride + x * 32] = 124 * 1024;
   }

   rand_pos_free(&pos);
   //-------------------------------------

   //Diamond-Square for filling in
   //-------------------------------------

   for(int l = -scale_log; l<5; l++)
   {
      int dim_level = 0;
      if(l>=0) dim_level = dim << l;
      else dim_level = dim >> (-l);

      int dim_rest = 0;
      if(l>=0) dim_rest = 32 >> l;
      else dim_rest = 32 << (-l);

#define diamond(ar, pre) \
        { \
           int32_t e0 = ar[(y) * dim_rest * stride + (x) * dim_rest]; \
           int32_t e1 = ar[(y) * dim_rest * stride + (x + 1) * dim_rest]; \
           int32_t e2 = ar[(y + 1) * dim_rest * stride + (x) * dim_rest]; \
           int32_t e3 = ar[(y + 1) * dim_rest * stride + (x + 1) * dim_rest]; \
           if(ar[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2]==UINT16_MAX) \
           { \
              int32_t e = (e0 + e1 + e2 + e3) / 4 + rand_offset(&rand, l, w->preset.pre); \
              ar[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2] = (uint16_t)RvR_max(0, RvR_min(e, UINT16_MAX - 1)); \
           } \
        }

      //Diamond
      for(int y = 0; y<dim_level; y++)
      {
         for(int x = 0; x<dim_level; x++)
         {
            diamond(wetness, var_rainfall);
            diamond(temperature, var_temperature);
            diamond(srf_top, var_elevation);
            diamond(sky_top, var_elevation);
            diamond(cv0_top, var_elevation);
            diamond(cv0_bot, var_elevation);
            diamond(cv1_top, var_elevation);
            diamond(cv1_bot, var_elevation);
            diamond(cv2_top, var_elevation);
            diamond(cv2_bot, var_elevation);
            /*
            int32_t e0 = elevation[(y) * dim_rest * stride + (x) * dim_rest];
            int32_t e1 = elevation[(y) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t e2 = elevation[(y + 1) * dim_rest * stride + (x) * dim_rest];
            int32_t e3 = elevation[(y + 1) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t t0 = temperature[(y) * dim_rest * stride + (x) * dim_rest];
            int32_t t1 = temperature[(y) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t t2 = temperature[(y + 1) * dim_rest * stride + (x) * dim_rest];
            int32_t t3 = temperature[(y + 1) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t r0 = rainfall[(y) * dim_rest * stride + (x) * dim_rest];
            int32_t r1 = rainfall[(y) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t r2 = rainfall[(y + 1) * dim_rest * stride + (x) * dim_rest];
            int32_t r3 = rainfall[(y + 1) * dim_rest * stride + (x + 1) * dim_rest];

            if(elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2]==-1)
            {
               elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2] = (e0 + e1 + e2 + e3) / 4 + rand_offset(&rand, l, w->preset.var_elevation);
               elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2] = RvR_max(0, elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2]);
            }
            if(temperature[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2]==-1)
               temperature[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2] = (t0 + t1 + t2 + t3) / 4 + rand_offset(&rand, l, w->preset.var_temperature);
            if(rainfall[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2]==-1)
               rainfall[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2] = (r0 + r1 + r2 + r3) / 4 + rand_offset(&rand, l, w->preset.var_rainfall);
            */
         }
      }
#undef diamond

#define square(ar, pre) \
        { \
           int32_t e0 = ar[(y) * dim_rest * stride + (x) * dim_rest]; \
           int32_t e1 = ar[(y) * dim_rest * stride + (x + 1) * dim_rest]; \
           int32_t e2 = ar[(y + 1) * dim_rest * stride + (x) * dim_rest]; \
           int32_t e3 = ar[(y + 1) * dim_rest * stride + (x + 1) * dim_rest]; \
           int32_t e4 = ar[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2]; \
           int32_t e5 = (e0 + e3 + e4) / 3; \
           int32_t e6 = (e0 + e1 + e4) / 3; \
           if(x>0) \
           e5 = ar[(y * dim_rest + dim_rest / 2) * stride + (x - 1) * dim_rest + dim_rest / 2]; \
           if(y>0) \
           e6 = ar[((y - 1) * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2]; \
           if(ar[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest]==UINT16_MAX) \
           { \
              int32_t e = (e0 + e3 + e5 + e4) / 4 + rand_offset(&rand, l, w->preset.pre); \
              ar[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest] = (uint16_t)RvR_max(0, RvR_min(UINT16_MAX - 1, e)); \
           } \
           if(ar[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2]==UINT16_MAX) \
           { \
              int32_t e = (e0 + e1 + e6 + e4) / 4 + rand_offset(&rand, l, w->preset.pre); \
              ar[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (uint16_t)RvR_max(0, RvR_min(UINT16_MAX - 1, e)); \
           } \
           if(x==dim_level - 1&&ar[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest]==UINT16_MAX) \
           { \
              int32_t e = (e1 + e2 + e4) / 3 + rand_offset(&rand, l, w->preset.pre); \
              ar[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest] = (uint16_t)RvR_max(0, RvR_min(UINT16_MAX - 1, e)); \
           } \
           if(y==dim_level - 1&&ar[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2]==UINT16_MAX) \
           { \
              int32_t e = (e2 + e3 + e4) / 3 + rand_offset(&rand, l, w->preset.pre); \
              ar[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (uint16_t)RvR_max(0, RvR_min(UINT16_MAX - 1, e)); \
           } \
        }

      //Square
      for(int y = 0; y<dim_level; y++)
      {
         for(int x = 0; x<dim_level; x++)
         {
            square(wetness, var_rainfall);
            square(temperature, var_temperature);
            square(srf_top, var_elevation);
            square(sky_top, var_elevation);
            square(cv0_top, var_elevation);
            square(cv0_bot, var_elevation);
            square(cv1_top, var_elevation);
            square(cv1_bot, var_elevation);
            square(cv2_top, var_elevation);
            square(cv2_bot, var_elevation);
            /*
            int32_t e0 = elevation[(y) * dim_rest * stride + (x) * dim_rest];
            int32_t e1 = elevation[(y) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t e2 = elevation[(y + 1) * dim_rest * stride + (x) * dim_rest];
            int32_t e3 = elevation[(y + 1) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t e4 = elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2];
            int32_t e5 = (e0 + e3 + e4) / 3;
            int32_t e6 = (e0 + e1 + e4) / 3;
            int32_t t0 = temperature[(y) * dim_rest * stride + (x) * dim_rest];
            int32_t t1 = temperature[(y) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t t2 = temperature[(y + 1) * dim_rest * stride + (x) * dim_rest];
            int32_t t3 = temperature[(y + 1) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t t4 = temperature[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2];
            int32_t t5 = (t0 + t3 + t4) / 3;
            int32_t t6 = (t0 + t1 + t4) / 3;
            int32_t r0 = rainfall[(y) * dim_rest * stride + (x) * dim_rest];
            int32_t r1 = rainfall[(y) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t r2 = rainfall[(y + 1) * dim_rest * stride + (x) * dim_rest];
            int32_t r3 = rainfall[(y + 1) * dim_rest * stride + (x + 1) * dim_rest];
            int32_t r4 = rainfall[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2];
            int32_t r5 = (r0 + r3 + r4) / 3;
            int32_t r6 = (r0 + r1 + r4) / 3;

            if(x>0)
            {
               e5 = elevation[(y * dim_rest + dim_rest / 2) * stride + (x - 1) * dim_rest + dim_rest / 2];
               t5 = temperature[(y * dim_rest + dim_rest / 2) * stride + (x - 1) * dim_rest + dim_rest / 2];
               r5 = rainfall[(y * dim_rest + dim_rest / 2) * stride + (x - 1) * dim_rest + dim_rest / 2];
            }
            if(y>0)
            {
               e6 = elevation[((y - 1) * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2];
               t6 = temperature[((y - 1) * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2];
               r6 = rainfall[((y - 1) * dim_rest + dim_rest / 2) * stride + x * dim_rest + dim_rest / 2];
            }

            if(elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest]==-1)
            {
               elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest] = (e0 + e3 + e5 + e4) / 4 + rand_offset(&rand, l, w->preset.var_elevation);
               elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest] = RvR_max(0, elevation[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest]);
            }
            if(temperature[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest]==-1)
               temperature[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest] = (t0 + t3 + t5 + t4) / 4 + rand_offset(&rand, l, w->preset.var_temperature);
            if(rainfall[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest]==-1)
               rainfall[(y * dim_rest + dim_rest / 2) * stride + x * dim_rest] = (r0 + r3 + r5 + r4) / 4 + rand_offset(&rand, l, w->preset.var_rainfall);

            if(elevation[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2]==-1)
            {
               elevation[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (e0 + e1 + e6 + e4) / 4 + rand_offset(&rand, l, w->preset.var_elevation);
               elevation[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2] = RvR_max(0, elevation[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2]);
            }
            if(temperature[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2]==-1)
               temperature[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (t0 + t1 + t6 + t4) / 4 + rand_offset(&rand, l, w->preset.var_temperature);
            if(rainfall[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2]==-1)
               rainfall[(y * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (r0 + r1 + r6 + r4) / 4 + rand_offset(&rand, l, w->preset.var_rainfall);

            if(x==dim_level - 1&&elevation[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest]==-1)
            {
               elevation[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest] = (e1 + e2 + e4) / 3 + rand_offset(&rand, l, w->preset.var_elevation);
               elevation[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest] = RvR_max(0, elevation[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest]);
            }
            if(x==dim_level - 1&&temperature[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest]==-1)
               temperature[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest] = (t1 + t2 + t4) / 3 + rand_offset(&rand, l, w->preset.var_temperature);
            if(x==dim_level - 1&&rainfall[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest]==-1)
               rainfall[(y * dim_rest + dim_rest / 2) * stride + (x + 1) * dim_rest] = (r1 + r2 + r4) / 3 + rand_offset(&rand, l, w->preset.var_rainfall);

            if(y==dim_level - 1&&elevation[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2]==-1)
            {
               elevation[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (e2 + e3 + e4) / 3 + rand_offset(&rand, l, w->preset.var_elevation);
               elevation[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2] = RvR_max(0, elevation[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2]);
            }
            if(y==dim_level - 1&&temperature[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2]==-1)
               temperature[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (t2 + t3 + t4) / 3 + rand_offset(&rand, l, w->preset.var_temperature);
            if(y==dim_level - 1&&rainfall[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2]==-1)
               rainfall[((y + 1) * dim_rest) * stride + x * dim_rest + dim_rest / 2] = (r2 + r3 + r4) / 3 + rand_offset(&rand, l, w->preset.var_rainfall);
            */
         }
      }

#undef square

#define resquare(ar, pre) \
        { \
           int32_t sum = 0; \
           int32_t count = 0; \
           if(x - 1>=0) {sum += ar[(y) * dim_rest * stride + (x) * dim_rest - dim_rest / 2]; count++;} \
           if(x + 1<dim_level) {sum += ar[(y) * dim_rest * stride + (x) * dim_rest + dim_rest / 2]; count++;} \
           if(y - 1>=0) {sum += ar[((y) * dim_rest - dim_rest / 2) * stride + (x) * dim_rest]; count++;} \
           if(y + 1<dim_level) {sum += ar[((y) * dim_rest + dim_rest / 2) * stride + (x) * dim_rest]; count++;} \
         \
           ar[y * dim_rest * stride + x * dim_rest] = (uint16_t)RvR_max(0, RvR_min(UINT16_MAX - 1, sum / count + rand_offset(&rand, l, w->preset.pre))); \
        }

      //Recalculate squares
      for(int y = 0; y<dim_level; y++)
      {
         for(int x = 0; x<dim_level; x++)
         {
            if((x * dim_rest) % (scale * 32)==0&&(y * dim_rest) % (scale * 32)==0&&l!=4)
               continue;

            resquare(srf_top, var_elevation);
            resquare(sky_top, var_elevation);
            resquare(cv0_top, var_elevation);
            resquare(cv0_bot, var_elevation);
            resquare(cv1_top, var_elevation);
            resquare(cv1_bot, var_elevation);
            resquare(cv2_top, var_elevation);
            resquare(cv2_bot, var_elevation);
            /*
            int32_t sum = 0;
            int32_t count = 0;
            if(x - 1>=0) {sum += elevation[(y) * dim_rest * stride + (x) * dim_rest - dim_rest / 2]; count++;}
            if(x + 1<dim_level) {sum += elevation[(y) * dim_rest * stride + (x) * dim_rest + dim_rest / 2]; count++;}
            if(y - 1>=0) {sum += elevation[((y) * dim_rest - dim_rest / 2) * stride + (x) * dim_rest]; count++;}
            if(y + 1<dim_level) {sum += elevation[((y) * dim_rest + dim_rest / 2) * stride + (x) * dim_rest]; count++;}

            elevation[y * dim_rest * stride + x * dim_rest] = sum / count + rand_offset(&rand, l, w->preset.var_elevation);
            */
         }
      }
   }
#undef resquare

   //-------------------------------------

   //Save to regions
   for(int y = 0; y<dim; y++)
   {
      for(int x = 0; x<dim; x++)
      {
         Region *r = region_create(w, x, y);
         RvR_mem_tag_set(r, RVR_MALLOC_STATIC);

         for(int ty = 0; ty<33; ty++)
         {
            for(int tx = 0; tx<33; tx++)
            {
               r->temperature[ty * 33 + tx] = temperature[(y * 32 + ty) * stride + x * 32 + tx];
               r->wetness[ty * 33 + tx] = wetness[(y * 32 + ty) * stride + x * 32 + tx];
               r->srf.top[ty * 33 + tx] = srf_top[(y * 32 + ty) * stride + x * 32 + tx];
               r->sky.top[ty * 33 + tx] = sky_top[(y * 32 + ty) * stride + x * 32 + tx];
               r->cv0.top[ty * 33 + tx] = cv0_top[(y * 32 + ty) * stride + x * 32 + tx];
               r->cv0.bot[ty * 33 + tx] = cv0_bot[(y * 32 + ty) * stride + x * 32 + tx];
               r->cv1.top[ty * 33 + tx] = cv1_top[(y * 32 + ty) * stride + x * 32 + tx];
               r->cv1.bot[ty * 33 + tx] = cv1_bot[(y * 32 + ty) * stride + x * 32 + tx];
               r->cv2.top[ty * 33 + tx] = cv2_top[(y * 32 + ty) * stride + x * 32 + tx];
               r->cv2.bot[ty * 33 + tx] = cv2_bot[(y * 32 + ty) * stride + x * 32 + tx];
            }
         }

         region_save(w, x, y);
         RvR_mem_tag_set(r, RVR_MALLOC_CACHE);
      }
   }

#if 0
   printf("P3\n%d %d\n255\n", dim * 32, dim * 32 * 2);
   for(int y = 0; y<dim * 32; y++)
   {
      for(int x = 0; x<dim * 32; x++)
      {
         int e = elevation[y * stride + x];
         int t = temperature[y * stride + x];
         int r = rainfall[y * stride + x];
         if(e<128 * 1024)
            printf("0 0 128\n");
         else
         {
            int biome_y = 5 - RvR_min(5, RvR_max(0, t / (16 * 1024)));
            int biome_x = RvR_min(7, RvR_max(0, r / (16 * 1024)));

            uint8_t colors[48][3] =
            {
               {255, 255, 128}, {224, 255, 128}, {192, 255, 128}, {160, 255, 128}, {128, 255, 128}, {96, 255, 128}, {64, 255, 144}, {32, 255, 160},
               {224, 224, 128}, {192, 224, 128}, {160, 224, 128}, {128, 224, 128}, {96, 224, 128}, {64, 224, 144}, {32, 224, 192}, {32, 224, 192},
               {192, 192, 128}, {160, 192, 128}, {128, 192, 128}, {96, 192, 128}, {64, 192, 144}, {32, 192, 192}, {32, 192, 192}, {32, 192, 192},
               {160, 160, 128}, {128, 160, 128}, {96, 160, 128}, {64, 160, 144}, {32, 160, 192}, {32, 160, 192}, {32, 160, 192}, {32, 160, 192},
               {128, 128, 128}, {96, 128, 128}, {64, 128, 144}, {32, 128, 192}, {32, 128, 192}, {32, 128, 192}, {32, 128, 192}, {32, 128, 192},
               {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}
            };

            printf("%d %d %d ", colors[biome_y * 8 + biome_x][0], colors[biome_y * 8 + biome_x][1], colors[biome_y * 8 + biome_x][2]);
         }
      }
   }

   for(int y = 0; y<dim * 32; y++)
   {
      for(int x = 0; x<dim * 32; x++)
      {
         int e = elevation[y * stride + x];
         if(e<128 * 1024)
            printf("0 0 128\n");
         else if(e>176 * 1024)
         {
            e = RvR_min(255, RvR_max(0, (e - 128 * 1024) / 512));
            printf("%d %d %d\n", e, e, e);
         }
         else if(e>136 * 1024)
         {
            e = RvR_min(255, RvR_max(0, (e - 128 * 1024) / 512));
            printf("%d %d %d\n", 0, e, 0);
         }
         else
         {
            e = RvR_min(255, RvR_max(0, (e - 64 * 1024) / 512));
            printf("%d %d %d\n", e, e, 0);
         }
      }
   }
#endif

   RvR_free(srf_top);
   RvR_free(sky_top);
   RvR_free(cv0_top);
   RvR_free(cv0_bot);
   RvR_free(cv1_top);
   RvR_free(cv1_bot);
   RvR_free(cv2_top);
   RvR_free(cv2_bot);
   RvR_free(wetness);
   RvR_free(temperature);
}

static int32_t rand_offset(RvR_rand_pcg *rand, int level, int32_t var)
{
   if(level>=0)
      return RvR_rand_pcg_next_range(rand, -var, var) / (1 << level);
   else
      return RvR_rand_pcg_next_range(rand, -var, var) * (1 << (-level));
}

static void rand_pos_init(int32_t **pos, World *w)
{
   unsigned dim = world_size_to_dim(w->size);
   for(int y = 1; y<dim - 1; y++)
   {
      for(int x = 1; x<dim - 1; x++)
      {
         RvR_array_push((*pos), x);
         RvR_array_push((*pos), y);
      }
   }
}

static void rand_pos_free(int32_t **pos)
{
   RvR_array_free(*pos);
}

static void rand_pos(RvR_rand_pcg *rand, int32_t **pos, int32_t *x, int32_t *y)
{
   int e = RvR_rand_pcg_next_range(rand, 0, (int32_t)RvR_array_length(*pos) / 2);

   *x = (*pos)[2 * e];
   *y = (*pos)[2 * e + 1];
   (*pos)[2 * e] = (*pos)[RvR_array_length(*pos) - 2];
   (*pos)[2 * e + 1] = (*pos)[RvR_array_length(*pos) - 1];

   RvR_array_length_set(*pos, RvR_array_length(*pos) - 2);
}
//-------------------------------------
