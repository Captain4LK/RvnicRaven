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
#include "util.h"
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

void region_file_load(World *w)
{
   RvR_rw rw = {0};
   w->region_file.offset = NULL;

   RvR_error_check(w!=NULL, "region_file_load", "world is null\n");

   int res = snprintf(w->region_file.path, UTIL_PATH_MAX, "%s/regions.dat", w->base_path);
   RvR_error_check(res<UTIL_PATH_MAX, "region_load", "region file name truncated, path too long\n");

   unsigned dim = world_size_to_dim(w->size);
   w->region_file.offset = RvR_malloc(sizeof(*w->region_file.offset) * dim * dim, "Region file offsets");

   //Read region file
   //-------------------------------------
   RvR_rw_init_path(&rw, w->region_file.path, "rb");

   uint32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0, "region_file_load", "version mismatch, expected version 0, got version %d\n", version);

   World_size size = RvR_rw_read_u32(&rw);
   RvR_error_check(size==w->size, "region_file_load", "size mismatch, region file is of size %d, but world of size %d\n", size, w->size);

   w->region_file.offset_next = RvR_rw_read_u32(&rw);

   for(unsigned i = 0; i<dim * dim; i++)
      w->region_file.offset[i] = RvR_rw_read_u32(&rw);

   RvR_rw_close(&rw);
   //-------------------------------------

RvR_err:
   if(w->region_file.offset!=NULL)
      RvR_free(w->region_file.offset);
   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   return;
}

void region_file_create(World *w)
{
   RvR_error_check(w!=NULL, "region_file_load", "world is null\n");

   int res = snprintf(w->region_file.path, UTIL_PATH_MAX, "%s/regions.dat", w->base_path);
   RvR_error_check(res<UTIL_PATH_MAX, "region_load", "region file name truncated, path too long\n");

   unsigned dim = world_size_to_dim(w->size);
   w->region_file.offset = RvR_malloc(sizeof(*w->region_file.offset) * dim * dim, "Region file offsets");
   for(unsigned i = 0; i<dim * dim; i++)
      w->region_file.offset[i] = -1;
   w->region_file.offset_next = 4 + 4 + 4 + dim * dim * 4;

   //Write region file
   //-------------------------------------
   RvR_rw rw;
   RvR_rw_init_path(&rw, w->region_file.path, "wb");

   //Version
   RvR_rw_write_u32(&rw, 0);

   //World dimension (for sanity check)
   RvR_rw_write_u32(&rw, w->size);

   //Offset of end of last region
   RvR_rw_write_u32(&rw, w->region_file.offset_next);

   //Offsets
   for(unsigned i = 0; i<dim * dim; i++)
      RvR_rw_write_u32(&rw, w->region_file.offset[i]);

   RvR_rw_close(&rw);
   //-------------------------------------

RvR_err:
   return;
}

Region *region_create(World *w, unsigned x, unsigned y)
{
   Region *r = NULL;

   RvR_error_check(w!=NULL, "region_create", "world is null\n");
   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim, "region_create", "region x pos (%d) out of bounds (%d)\n", x, dim);
   RvR_error_check(y<dim, "region_create", "region y pos (%d) out of bounds (%d)\n", y, dim);

   r = RvR_malloc(sizeof(*r), "Region struct");
   memset(r, 0, sizeof(*r));

   w->regions[x * dim + y] = r;
   RvR_mem_tag_set(r, RVR_MALLOC_CACHE);
   RvR_mem_usr_set(r, (void **)&w->regions[x * dim + y]);
   return r;

RvR_err:
   if(r!=NULL)
      RvR_free(r);

   return NULL;
}

Region *region_load(World *w, unsigned x, unsigned y)
{
   //TODO
   return NULL;
}

void region_save(World *w, unsigned x, unsigned y)
{
   //TODO
}

/*Region *region_create(World *w, unsigned x, unsigned y)
{
   Region *r = NULL;

   RvR_error_check(w!=NULL,"region_create","world is null\n");
   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim,"region_create","region x pos (%d) out of bounds (%d)\n",x,dim);
   RvR_error_check(y<dim,"region_create","region y pos (%d) out of bounds (%d)\n",y,dim);

   r = RvR_malloc(sizeof(*r),"Region struct");
   for(int i = 0;i<16*16*8;i++)
      r->offset[i] = -1;
   r->offset_next = 4+4+16*16*8*4;
   int res = snprintf(r->path,UTIL_PATH_MAX,"%s/%03x-%03x.dat",w->base_path,x,y);
   RvR_error_check(res<UTIL_PATH_MAX,"region_create","region file name truncated, path too long\n");

   //Write initial region file
   //-------------------------------------
   RvR_rw rw;
   RvR_rw_init_path(&rw,r->path,"wb");

   //Version
   RvR_rw_write_u32(&rw,0);

   //Address of end of last chunk
   RvR_rw_write_u32(&rw,r->offset_next);

   //Chunk addresses
   for(int i = 0;i<16*16*8;i++)
      RvR_rw_write_u32(&rw,r->offset[i]);

   RvR_rw_close(&rw);
   //-------------------------------------

   w->regions[x*dim+y] = r;
   RvR_mem_tag_set(r,RVR_MALLOC_CACHE);
   RvR_mem_usr_set(r,(void **)&w->regions[x*dim+y]);
   return r;

RvR_err:
   if(r!=NULL)
      RvR_free(r);

   return NULL;
}

Region *region_load(World *w, unsigned x, unsigned y, int create_if_missing)
{
   Region *r = NULL;
   RvR_rw rw = {0};

   RvR_error_check(w!=NULL,"region_load","world is null\n");
   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim,"region_load","region x pos (%d) out of bounds (%d)\n",x,dim);
   RvR_error_check(y<dim,"region_load","region y pos (%d) out of bounds (%d)\n",y,dim);

   r = RvR_malloc(sizeof(*r),"Region struct");
   int res = snprintf(r->path,UTIL_PATH_MAX,"%s/%03x-%03x.dat",w->base_path,x,y);
   RvR_error_check(res<UTIL_PATH_MAX,"region_load","region file name truncated, path too long\n");

   RvR_rw_init_path(&rw,r->path,"rb");
   RvR_error_check(RvR_rw_valid(&rw)||create_if_missing,"region_load","can't read region file '%s'\n",r->path);
   if(!RvR_rw_valid(&rw))
   {
      RvR_free(r);
      return region_create(w,x,y);
   }

   //Read region file
   //-------------------------------------
   uint32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0,"region_load","version mismatch, got version %d, expected version 0\n",version);
   r->offset_next = RvR_rw_read_u32(&rw);
   for(int i = 0;i<16*16*8;i++)
      r->offset[i] = RvR_rw_read_u32(&rw);
   //-------------------------------------

   w->regions[x*dim+y] = r;
   RvR_mem_tag_set(r,RVR_MALLOC_CACHE);
   RvR_mem_usr_set(r,(void **)&w->regions[x*dim+y]);
   return r;

RvR_err:
   if(r!=NULL)
      RvR_free(r);
   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   return NULL;
}*/
//-------------------------------------
