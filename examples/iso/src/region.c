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
static uint8_t *region_buffer = NULL;
static size_t region_buffer_size = 0;
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

Region *region_get(World *w, unsigned x, unsigned y)
{
   Region *r = NULL;
   RvR_rw rw = {0};
   RvR_rw rw_decomp = {0};
   RvR_rw rw_reg = {0};
   uint8_t *mem_decomp = NULL;

   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim,"region_get","region x position %u out of bounds, dimension is %u\n",x,dim);
   RvR_error_check(y<dim,"region_get","region y position %u out of bounds, dimension is %u\n",y,dim);

   //Still loaded
   if(w->regions[y*dim+x]!=NULL)
      return w->regions[y*dim+x];

   //Check region offset
   int32_t offset = w->region_file.offset[y*dim+x];
   RvR_error_check(offset>=0,"region_get","region %u,%u does not exist in region file, file might be corrupt\n",x,y);

   RvR_rw_init_path(&rw,w->region_file.path,"rb");

   //Read block
   //-------------------------------------
   RvR_rw_seek(&rw,offset,SEEK_SET);
   int32_t size = RvR_rw_read_u32(&rw);
   RvR_error_check(size>=0,"region_get","size of region %u,%u is negative, file might be corrupt\n",size);

   if(region_buffer==NULL||region_buffer_size<size)
   {
      region_buffer = RvR_realloc(region_buffer,size,"Region buffer");
      region_buffer_size = size;
   }

   RvR_mem_tag_set(region_buffer,RVR_MALLOC_STATIC);
   RvR_mem_usr_set(region_buffer,(void **)&region_buffer);
   RvR_rw_read(&rw,region_buffer,1,size);
   RvR_rw_close(&rw);
   //-------------------------------------

   //Decompress and read region
   //-------------------------------------
   RvR_rw_init_const_mem(&rw_decomp,region_buffer,size);
   int32_t size_out = 0;
   mem_decomp = RvR_crush_decompress(&rw_decomp,&size_out);
   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);

   RvR_rw_init_const_mem(&rw_reg,mem_decomp,size_out);

   r = RvR_malloc(sizeof(*r),"Region struct");
   memset(r,0,sizeof(*r));

   //Read tiles
   for(int i = 0;i<32*32;i++)
      r->tiles[i] = RvR_rw_read_u16(&rw_reg);

   RvR_rw_close(&rw_reg);
   //-------------------------------------

   w->regions[y*dim+x] = r;
   RvR_mem_tag_set(r,RVR_MALLOC_CACHE);
   RvR_mem_usr_set(r,(void**)&w->regions[y*dim+x]);

   //Changing tags to cache needs to be done last!
   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_CACHE);
   RvR_mem_tag_set(region_buffer,RVR_MALLOC_CACHE);

   return r;

RvR_err:
   if(r!=NULL)
      RvR_free(r);

   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   if(RvR_rw_valid(&rw_decomp))
      RvR_rw_close(&rw_decomp);

   if(RvR_rw_valid(&rw_reg))
      RvR_rw_close(&rw_reg);

   //Changing tags to cache needs to be done last!
   if(region_buffer!=NULL)
      RvR_mem_tag_set(region_buffer,RVR_MALLOC_CACHE);

   if(mem_decomp!=NULL)
      RvR_mem_tag_set(mem_decomp,RVR_MALLOC_CACHE);

   return NULL;
}

void region_save(World *w, unsigned x, unsigned y)
{
   //TODO
}
//-------------------------------------
