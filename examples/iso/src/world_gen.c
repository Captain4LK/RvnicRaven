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
static int32_t rand_offset(RvR_rand_pcg *rand, int level);
static void rand_pos_init(int32_t **pos, World *w);
static void rand_pos_free(int32_t **pos);
static void rand_pos(RvR_rand_pcg *rand, int32_t **pos, int32_t *x, int32_t *y);
//-------------------------------------

//Function implementations

void world_gen(World *w, uint32_t seed, WorldGen_preset *preset)
{
   RvR_rand_pcg rand = {0};
   RvR_rand_pcg_seed(&rand,seed);

   unsigned dim = world_size_to_dim(w->size);
   int stride = dim*32+1;
   int32_t *elevation = RvR_malloc(sizeof(*elevation)*stride*stride,"Map Gen elevation");
   for(int i = 0;i<stride*stride;i++)
   {
      elevation[i] = -1;
   }

   //Basic parameters
   //-------------------------------------

   //Init with random elevation above sea level
   //TODO(Captain4LK): weigh with circle?
   for(int y = 0;y<=dim;y++)
   {
      for(int x = 0;x<=dim;x++)
      {
         //Sea level at 128*1024
         elevation[y*32*stride+x*32] = RvR_rand_pcg_next_range(&rand,129*1024,140*1024);
      }
   }
  
   //Corners are below sea level
   for(int i = 0;i<stride;i++)
   {
      elevation[i] = 64*1024;
      elevation[(stride-1)*stride+i] = 64*1024;
      elevation[i*stride] = 64*1024;
      elevation[i*stride+stride-1] = 64*1024;
   }

   int32_t *pos = NULL;
   rand_pos_init(&pos,w);

   //Place up to n mountainpeaks
   for(int i = 0;i<preset->mountains_high;i++)
   {
      int x,y;
      rand_pos(&rand,&pos,&x,&y);
      elevation[y*32*stride+x*32] = 196*1024-RvR_rand_pcg_next_range(&rand,-8192,8192);
   }

   //Place n deep lakes
   for(int i = 0;i<preset->lakes_deep;i++)
   {
      int x,y;
      rand_pos(&rand,&pos,&x,&y);
      elevation[y*32*stride+x*32] = 64*1024;
   }

   //Place n shallow lakes
   for(int i = 0;i<preset->lakes_shallow;i++)
   {
      int x = RvR_rand_pcg_next_range(&rand,0,dim);
      int y = RvR_rand_pcg_next_range(&rand,0,dim);
      elevation[y*32*stride+x*32] = 124*1024;
   }

   rand_pos_free(&pos);
   //-------------------------------------

   //Diamond-Square for filling in
   //-------------------------------------

   for(int l = 0;l<5;l++)
   {
      int dim_level = dim<<l;
      int dim_rest = 32>>l;

      //Diamond
      for(int y = 0;y<dim_level;y++)
      {
         for(int x = 0;x<dim_level;x++)
         {
            int32_t c0 = elevation[(y)*dim_rest*stride+(x)*dim_rest];
            int32_t c1 = elevation[(y)*dim_rest*stride+(x+1)*dim_rest];
            int32_t c2 = elevation[(y+1)*dim_rest*stride+(x)*dim_rest];
            int32_t c3 = elevation[(y+1)*dim_rest*stride+(x+1)*dim_rest];

            //Diamond
            if(elevation[(y*dim_rest+dim_rest/2)*stride+x*dim_rest+dim_rest/2]==-1)
               elevation[(y*dim_rest+dim_rest/2)*stride+x*dim_rest+dim_rest/2] = (c0+c1+c2+c3)/4+rand_offset(&rand,l);
         }
      }

      //Square
      for(int y = 0;y<dim_level;y++)
      {
         for(int x = 0;x<dim_level;x++)
         {
            int32_t c0 = elevation[(y)*dim_rest*stride+(x)*dim_rest];
            int32_t c1 = elevation[(y)*dim_rest*stride+(x+1)*dim_rest];
            int32_t c2 = elevation[(y+1)*dim_rest*stride+(x)*dim_rest];
            int32_t c3 = elevation[(y+1)*dim_rest*stride+(x+1)*dim_rest];
            int32_t d4 = elevation[(y*dim_rest+dim_rest/2)*stride+x*dim_rest+dim_rest/2];
            int32_t d0 = (c0+c3+d4)/3;
            int32_t d1 = (c0+c1+d4)/3;
            int32_t d2 = (c1+c2+d4)/3;
            int32_t d3 = (c2+c3+d4)/3;
            if(x>0)
               d0 = elevation[(y*dim_rest+dim_rest/2)*stride+(x-1)*dim_rest+dim_rest/2];
            if(y>0)
               d1 = elevation[((y-1)*dim_rest+dim_rest/2)*stride+x*dim_rest+dim_rest/2];
            if(x<dim_level-1)
               d2 = elevation[(y*dim_rest+dim_rest/2)*stride+(x+1)*dim_rest+dim_rest/2];
            if(y<dim_level-1)
               d3 = elevation[((y+1)*dim_rest+dim_rest/2)*stride+x*dim_rest+dim_rest/2];

            //Square
            if(elevation[(y*dim_rest+dim_rest/2)*stride+x*dim_rest]==-1)
               elevation[(y*dim_rest+dim_rest/2)*stride+x*dim_rest] = (c0+c3+d0+d4)/4+rand_offset(&rand,l);
            if(elevation[(y*dim_rest)*stride+x*dim_rest+dim_rest/2]==-1)
               elevation[(y*dim_rest)*stride+x*dim_rest+dim_rest/2] = (c0+c1+d1+d4)/4+rand_offset(&rand,l);

            if(x==dim_level-1&&elevation[(y*dim_rest+dim_rest/2)*stride+(x+1)*dim_rest]==-1)
               elevation[(y*dim_rest+dim_rest/2)*stride+(x+1)*dim_rest] = (c1+c2+d4)/3+rand_offset(&rand,l);
            if(y==dim_level-1&&elevation[((y+1)*dim_rest)*stride+x*dim_rest+dim_rest/2]==-1)
               elevation[((y+1)*dim_rest)*stride+x*dim_rest+dim_rest/2] = (c2+c3+d4)/3+rand_offset(&rand,l);
         }
      }
   }

   //-------------------------------------

   //Save to regions
   for(int y = 0;y<dim;y++)
   {
      for(int x = 0;x<dim;x++)
      {
         Region *r = region_create(w,x,y);
         RvR_mem_tag_set(r,RVR_MALLOC_STATIC);

         for(int ty = 0;ty<33;ty++)
         {
            for(int tx = 0;tx<33;tx++)
            {
               r->elevation[ty*33+tx] = elevation[(y*32+ty)*stride+x*32+tx];
            }
         }

         region_save(w,x,y);
         RvR_mem_tag_set(r,RVR_MALLOC_CACHE);
      }
   }

   printf("P3\n%d %d\n255\n",dim*32,dim*32);
   for(int y = 0;y<dim*32;y++)
   {
      for(int x = 0;x<dim*32;x++)
      {
         int e = elevation[y*stride+x];
         if(e<128*1024)
            printf("0 0 128\n");
         else
         {
            e = RvR_min(255,RvR_max(0,(e-128*1024)/512));
            printf("%d %d %d\n",e,e,e);
         }
         //printf("%c",elevation[y*stride+x]>128*1024?'#':' ');
      }
   }

   RvR_free(elevation);
}

static int32_t rand_offset(RvR_rand_pcg *rand, int level)
{
   return RvR_rand_pcg_next_range(rand,-8192,8192)/(1<<level);
}

static void rand_pos_init(int32_t **pos, World *w)
{
   unsigned dim = world_size_to_dim(w->size);
   for(int y = 1;y<dim-1;y++)
   {
      for(int x = 1;x<dim-1;x++)
      {
         RvR_array_push((*pos),x);
         RvR_array_push((*pos),y);
      }
   }
}

static void rand_pos_free(int32_t **pos)
{
   RvR_array_free(*pos);
}

static void rand_pos(RvR_rand_pcg *rand, int32_t **pos, int32_t *x, int32_t *y)
{
   int e = RvR_rand_pcg_next_range(rand,0,RvR_array_length(*pos)/2);

   *x = (*pos)[2*e];
   *y = (*pos)[2*e+1];
   (*pos)[2*e] = (*pos)[RvR_array_length(*pos)-2];
   (*pos)[2*e+1] = (*pos)[RvR_array_length(*pos)-1];

   RvR_array_length_set(*pos,RvR_array_length(*pos)-2);
}
//-------------------------------------
