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
#include "chunk.h"
#include "region.h"
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

Chunk *chunk_generate(World *w, unsigned x, unsigned y, unsigned z)
{
   Chunk *c = NULL;

   RvR_error_check(w!=NULL,"chunk_generate","world is null\n");
   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim*16,"chunk_generate","chunk x pos (%d) out of bounds (%d)\n",x,dim*16);
   RvR_error_check(y<dim*16,"chunk_generate","chunk y pos (%d) out of bounds (%d)\n",y,dim*16);
   RvR_error_check(z<8,"chunk_generate","chunk z pos (%d) out of bounds (%d)\n",z,8);

   c = RvR_malloc(sizeof(*c),"Chunk struct");
   memset(c,0,sizeof(*c));
   c->x = x;
   c->y = y;
   c->z = z;

   return c;

RvR_err:
   if(c!=NULL)
      RvR_free(c);

   return NULL;
}

Chunk *chunk_load(World *w, unsigned x, unsigned y, unsigned z, int generate_if_missing)
{
   Chunk *c = NULL;

   RvR_error_check(w!=NULL,"chunk_load","world is null\n");
   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim*16,"chunk_load","chunk x pos (%d) out of bounds (%d)\n",x,dim*16);
   RvR_error_check(y<dim*16,"chunk_load","chunk y pos (%d) out of bounds (%d)\n",y,dim*16);
   RvR_error_check(z<8,"chunk_load","chunk z pos (%d) out of bounds (%d)\n",z,8);

   //TODO

   return c;

RvR_err:
   if(c!=NULL)
      RvR_free(c);

   return NULL;
}
//-------------------------------------
