/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "area.h"
#include "world.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Area *area_create(World *w, uint16_t x, uint16_t y, uint8_t dimx, uint8_t dimy, uint8_t dimz, uint16_t id)
{
   Area *a = NULL;

   RvR_error_check(w!=NULL,"area_create","world is null\n");

   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim*32,"area_create","area x pos (%d) out of bounds (%d)\n",x,dim*32);
   RvR_error_check(y<dim*32,"area_create","area y pos (%d) out of bounds (%d)\n",y,dim*32);
   RvR_error_check(dimx>0,"area_create","area x dimension must be more than zero\n");
   RvR_error_check(dimy>0,"area_create","area y dimension must be more than zero\n");
   RvR_error_check(dimz>0,"area_create","area z dimension must be more than zero\n");

   a = RvR_malloc(sizeof(*a),"Area struct");
   a->dimx = dimx;
   a->dimy = dimy;
   a->dimz = dimz;
   a->id = id;
   a->tiles = NULL;
   a->tiles = RvR_malloc(sizeof(*a->tiles)*(dimx*32)*(dimy*32)*(dimz*32),"Area tiles");

   for(int i = 0;i<32*32*32;i++)
      a->tiles[i] = 1 | (1<<14);

   return a;

RvR_err:
   if(a!=NULL)
   {
      RvR_free(a);
      if(a->tiles!=NULL)
         RvR_free(a->tiles);
   }

   return NULL;
}

Area *area_load(World *w, uint16_t id)
{
   Area *a = NULL;

   //TODO

   return a;

RvR_err:
   if(a!=NULL)
      RvR_free(a);

   return NULL;
}

uint32_t area_tile(const Area *a, int x, int y, int z)
{
   if(x<0||y<0||z<0)
      return 0;
   
   if(x>=a->dimx*32||y>=a->dimy*32||z>=a->dimz*32)
      return 0;

   return a->tiles[z*32*32+y*32+x];
}
//-------------------------------------