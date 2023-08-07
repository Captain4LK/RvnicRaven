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
#include "tile.h"
#include "world.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t *area_buffer = NULL;
static int32_t area_buffer_size = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Area *area_create(World *w, uint16_t x, uint16_t y, uint8_t dimx, uint8_t dimy, uint8_t dimz, uint16_t id)
{
   Area *a = NULL;

   RvR_error_check(w!=NULL, "area_create", "world is null\n");

   unsigned dim = world_size_to_dim(w->size);
   RvR_error_check(x<dim * 32, "area_create", "area x pos (%d) out of bounds (%d)\n", x, dim * 32);
   RvR_error_check(y<dim * 32, "area_create", "area y pos (%d) out of bounds (%d)\n", y, dim * 32);
   RvR_error_check(dimx>0, "area_create", "area x dimension must be more than zero\n");
   RvR_error_check(dimy>0, "area_create", "area y dimension must be more than zero\n");
   RvR_error_check(dimz>0, "area_create", "area z dimension must be more than zero\n");

   a = RvR_malloc(sizeof(*a), "Area struct");
   a->dimx = dimx;
   a->dimy = dimy;
   a->dimz = dimz;
   a->id = id;
   a->entities = NULL;
   a->entity_grid = RvR_malloc(sizeof(*a->entity_grid) * (dimx * 4) * (dimy * 4) * (dimz * 4), "Area entity grid");
   memset(a->entity_grid, 0, sizeof(*a->entity_grid) * (dimx * 4) * (dimy * 4) * (dimz * 4));
   a->tiles = NULL;
   a->tiles = RvR_malloc(sizeof(*a->tiles) * (dimx * 32) * (dimy * 32) * (dimz * 32), "Area tiles");

   for(int i = 0; i<dimx * 32 * dimy * 32 * dimz * 32; i++)
      a->tiles[i] = tile_set_discovered(0, 0, 0);

   for(int z = 16; z<dimz * 32; z++)
   {
      for(int x = 0; x<dimx * 32; x++) for(int y = 0; y<dimy * 32; y++)
         {
            area_set_tile(a, x, y, z, tile_set_discovered(tile_make_wall(1, 1), 0, 0));
         }
   }

   for(int x = 0; x<16; x++) for(int y = 0; y<8; y++)
         area_set_tile(a, x + 8, y + 8, 15, tile_set_discovered(tile_make_wall(1, 1), 0, 0));
   for(int x = 0; x<6; x++) for(int y = 0; y<8; y++)
         area_set_tile(a, x + 15, y + 16, 15, tile_set_discovered(tile_make_wall(1, 1), 0, 0));
   for(int x = 0; x<14; x++) for(int y = 0; y<6; y++)
         area_set_tile(a, x + 9, y + 9, 14, tile_set_discovered(tile_make_wall(1, 1), 0, 0));
   area_set_tile(a, 8, 14, 14, tile_set_discovered(tile_make_slope(1, 0), 0, 0));
   area_set_tile(a, 8, 15, 14, tile_set_discovered(tile_make_slope(1, 8), 0, 0));
   area_set_tile(a, 9, 15, 14, tile_set_discovered(tile_make_slope(1, 1), 0, 0));

   area_set_tile(a, 1, 1, 15, tile_set_discovered(tile_make_slope(1, 13), 0, 0));

   area_set_tile(a, 3, 3, 15, tile_set_discovered(tile_make_wall(1, 1), 0, 0));
   //area_set_tile(a,3,4,15,tile_set_discovered(tile_make_wall(1,1),1));

   for(int x = 0; x<16; x++) for(int y = 0; y<8; y++)
         area_set_tile(a, x + 8, y + 32, 16, tile_set_discovered(tile_make_wall(0, 0), 0, 0));
   for(int x = 0; x<16; x++) for(int y = 0; y<8; y++)
         area_set_tile(a, x + 8, y + 32, 17, tile_set_discovered(tile_make_wall(0, 0), 0, 0));
   for(int x = 0; x<16; x++) for(int y = 0; y<8; y++)
         area_set_tile(a, x + 8, y + 32, 18, tile_set_discovered(tile_make_wall(0, 0), 0, 0));

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
   Region *r = NULL;
   RvR_rw rw = {0};
   RvR_rw rw_decomp = {0};
   RvR_rw rw_reg = {0};
   uint8_t *mem_decomp = NULL;
   Area *a = NULL;

   RvR_error_check(w!=NULL,"area_load","world is null\n");

   int file_id = id/64;
   char path[UTIL_PATH_MAX];
   int res = snprintf(path,UTIL_PATH_MAX,"%s/regions%05d.dat",w->base_path,file_id);
   RvR_error_check(res<UTIL_PATH_MAX, "area_load", "area file name truncated, path too long\n");
   RvR_rw_init_path(&rw,path,"rb");
   RvR_error_check(RvR_rw_valid(&rw),"area_load","failed to open file '%s'\n",path);

   int32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0,"area_load","version mismatch, expected version 0, got version %d\n",version);
   RvR_rw_seek(&rw,8+(file_id*64-id)*4,SEEK_SET);
   int32_t offset = RvR_rw_read_u32(&rw);
   RvR_error_check(offset>=0,"area_load","offset is negative, area not created yet\n");

   RvR_rw_seek(&rw,offset,SEEK_SET);
   int32_t size = RvR_rw_read_u32(&rw);
   RvR_error_check(size>=0,"area_load","size of area %u is negative, file might be corrupt\n",id);

   if(area_buffer==NULL||area_buffer_size<size)
   {
      area_buffer = RvR_realloc(area_buffer,size,"Area buffer");
      area_buffer_size = size;
   }

   RvR_mem_tag_set(area_buffer,RVR_MALLOC_STATIC);
   RvR_mem_usr_set(area_buffer,(void **)&area_buffer);
   RvR_rw_read(&rw,area_buffer,1,size);
   RvR_rw_close(&rw);

   RvR_rw_init_const_mem(&rw_decomp,area_buffer,size);
   int32_t size_out = 0;
   mem_decomp = RvR_crush_decompress(&rw_decomp,&size_out);
   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);

   RvR_rw_init_const_mem(&rw_reg,mem_decomp,size_out);

   //Create region from data
   //-------------------------------------
   r = RvR_malloc(sizeof(*r),"Region struct");
   memset(r,0,sizeof(*r));

   a = RvR_malloc(sizeof(*a),"Area struct");
   a->id = id;
   a->dimx = RvR_rw_read_u16(&rw_reg);
   a->dimy = RvR_rw_read_u16(&rw_reg);
   a->dimz = RvR_rw_read_u16(&rw_reg);
   a->entities = NULL;
   a->entity_grid = RvR_malloc(sizeof(*a->entity_grid) * (a->dimx * 4) * (a->dimy * 4) * (a->dimz * 4), "Area entity grid");
   memset(a->entity_grid, 0, sizeof(*a->entity_grid) * (a->dimx * 4) * (a->dimy * 4) * (a->dimz * 4));
   a->tiles = RvR_malloc(sizeof(*a->tiles)*(a->dimx*32)*(a->dimy*32)*(a->dimz*32),"Area tiles");
   for(int i = 0;i<(a->dimx*32)*(a->dimy*32)*(a->dimz*32);i++)
      a->tiles[i] = RvR_rw_read_u32(&rw_reg);
   //-------------------------------------

   RvR_rw_close(&rw_reg);
   //Changing tags to cache needs to be done last!
   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_CACHE);
   RvR_mem_tag_set(area_buffer,RVR_MALLOC_CACHE);

   return a;

RvR_err:
   if(a!=NULL)
   {
      if(a->tiles!=NULL)
         RvR_free(a->tiles);
      RvR_free(a);
   }

   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   if(RvR_rw_valid(&rw_decomp))
      RvR_rw_close(&rw_decomp);

   if(RvR_rw_valid(&rw_reg))
      RvR_rw_close(&rw_reg);

   //Changing tags to cache needs to be done last!
   if(area_buffer!=NULL)
      RvR_mem_tag_set(area_buffer,RVR_MALLOC_CACHE);

   if(mem_decomp!=NULL)
      RvR_mem_tag_set(mem_decomp,RVR_MALLOC_CACHE);

   return NULL;
}

void area_save(World *w, Area *a)
{
   RvR_rw rw = {0};
   RvR_rw rw_comp = {0};
   RvR_rw rw_comp_out = {0};

   RvR_error_check(w!=NULL,"area_save","world is null\n");
   RvR_error_check(a!=NULL,"area_save","area is null\n");

   uint8_t *comp_out = NULL;
   int file_id = a->id/64;
   char path[UTIL_PATH_MAX];
   int res = snprintf(path,UTIL_PATH_MAX,"%s/regions%05d.dat",w->base_path,file_id);
   RvR_error_check(res<UTIL_PATH_MAX, "area_load", "area file name truncated, path too long\n");

   //Read offsets
   RvR_rw_init_path(&rw,path,"rb+");
   if(!RvR_rw_valid(&rw))
   {
      //File doesn't exist
      RvR_rw_init_path(&rw,path,"wb");
      RvR_error_check(RvR_rw_valid(&rw),"area_save","failed to create file '%s'\n",path);

      RvR_rw_write_u32(&rw,0);
      RvR_rw_write_u32(&rw,264);
      for(int i = 0;i<64;i++)
         RvR_rw_write_u32(&rw,-1);

      RvR_rw_close(&rw);
      RvR_rw_init_path(&rw,path,"rb+");
   }

   uint32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0,"area_save","version mismatch, expected version 0, got version %d\n",version);

   int32_t offset_next = RvR_rw_read_u32(&rw);
   int32_t offsets[64];
   for(int i = 0;i<64;i++)
      offsets[i] = RvR_rw_read_u32(&rw);
   int32_t offset = offsets[file_id*64-a->id];

   //Compress
   //-------------------------------------
   int32_t size = (a->dimx*32)*(a->dimy*32)*(a->dimz*32)*4;
   size+=6;

   if(area_buffer==NULL||area_buffer_size<size)
   {
      area_buffer = RvR_realloc(area_buffer,size,"Area buffer");
      area_buffer_size = size;
      RvR_mem_usr_set(area_buffer,(void **)&area_buffer);
   }

   RvR_mem_tag_set(area_buffer,RVR_MALLOC_STATIC);
   RvR_rw_init_mem(&rw_comp,area_buffer,size,0);

   //Write area data
   RvR_rw_write_u16(&rw_comp,a->dimx);
   RvR_rw_write_u16(&rw_comp,a->dimy);
   RvR_rw_write_u16(&rw_comp,a->dimz);
   for(int i = 0;i<(a->dimx*32)*(a->dimy*32)*(a->dimz*32);i++)
      RvR_rw_write_u32(&rw_comp,a->tiles[i]);

   RvR_rw_init_dyn_mem(&rw_comp_out,size,1);
   RvR_crush_compress(&rw_comp,&rw_comp_out,10);
   RvR_rw_seek(&rw_comp_out,0,SEEK_END);
   size = RvR_rw_tell(&rw_comp_out);
   comp_out = rw_comp_out.as.dmem.mem;

   RvR_rw_close(&rw_comp);
   RvR_mem_tag_set(area_buffer,RVR_MALLOC_CACHE);
   //-------------------------------------

   //Get old size of block
   RvR_rw_seek(&rw,offset,SEEK_SET);
   int32_t size_old = RvR_rw_read_u32(&rw);

   //Adjust indices
   //-------------------------------------
   if(offset==-1)
   {
      offset = offset_next;
      offset_next+=size+4;
      offsets[file_id*64-a->id] = offset;
   }
   else
   {
      for(int i = 0;i<64;i++)
         if(offsets[i]>offset)
            offsets[i]+=size-size_old;
      offset_next+=size-size_old;
   }

   //Rewrite indices in file
   RvR_rw_seek(&rw,4,SEEK_SET);
   RvR_rw_write_u32(&rw,offset_next);
   for(int i = 0;i<64;i++)
      RvR_rw_write_u32(&rw,offsets[i]);
   //-------------------------------------

   //Move blocks
   //-------------------------------------
   if(size>size_old)
   {
      //Move data by (size-old_size) bytes
      RvR_rw_seek(&rw,0,SEEK_END);
      int32_t file_size_old = RvR_rw_tell(&rw);
      int32_t file_size_new = file_size_old+(size-size_old);

      util_truncate(&rw,file_size_new);

      int32_t to_move = file_size_old-offset;
      int32_t pos_read = file_size_old;
      int32_t pos_write = file_size_new;
      for(int i = 0;i<=to_move/4096;i++)
      {
         int32_t block = RvR_min(4096,pos_read-offset);
         uint8_t buffer[4096];

         //Read
         RvR_rw_seek(&rw,pos_read-block,SEEK_SET);
         RvR_rw_read(&rw,buffer,block,1);

         //Write
         RvR_rw_seek(&rw,pos_write-block,SEEK_SET);
         RvR_rw_write(&rw,buffer,block,1);

         pos_read-=block;
         pos_write-=block;
      }
   }
   else if(size<size_old)
   {
      //Move data by (size-old_size) bytes
      //Different to above, since we need to move from left
      //instead of from right
      RvR_rw_seek(&rw,0,SEEK_END);
      int32_t file_size_old = RvR_rw_tell(&rw);
      int32_t file_size_new = file_size_old+(size-size_old);

      int32_t to_move = file_size_old-offset;
      int32_t pos_read = offset+size_old;
      int32_t pos_write = offset+size;
      for(int i = 0;i<=to_move/4096;i++)
      {
         int32_t block = RvR_min(4096,file_size_old-pos_read);
         uint8_t buffer[4096];

         //Read
         RvR_rw_seek(&rw,pos_read+block,SEEK_SET);
         RvR_rw_read(&rw,buffer,block,1);

         //Write
         RvR_rw_seek(&rw,pos_write+block,SEEK_SET);
         RvR_rw_write(&rw,buffer,block,1);

         pos_read+=block;
         pos_write+=block;
      }

      util_truncate(&rw,file_size_new);
   }
   //-------------------------------------

   //Write new data
   RvR_rw_seek(&rw,offset,SEEK_SET);
   RvR_rw_write_u32(&rw,size);
   RvR_rw_write(&rw,comp_out,1,size);

   RvR_rw_close(&rw_comp_out);
   RvR_rw_close(&rw);

   return;

RvR_err:
   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   if(RvR_rw_valid(&rw_comp))
      RvR_rw_close(&rw_comp);

   if(RvR_rw_valid(&rw_comp_out))
      RvR_rw_close(&rw_comp_out);

   //Changing tags to cache needs to be done last!
   if(area_buffer!=NULL)
      RvR_mem_tag_set(area_buffer,RVR_MALLOC_CACHE);

   return;
}

uint32_t area_tile(const Area *a, int x, int y, int z)
{
   if(x<0||y<0||z<0)
      return tile_set_discovered(0, 1, 1);

   if(x>=a->dimx * 32||y>=a->dimy * 32||z>=a->dimz * 32)
      return tile_set_discovered(0, 1, 1);

   return a->tiles[z * a->dimx * 32 * a->dimy * 32 + y * a->dimx * 32 + x];
}

void area_set_tile(Area *a, int x, int y, int z, uint32_t tile)
{
   if(x<0||y<0||z<0)
      return;

   if(x>=a->dimx * 32||y>=a->dimy * 32||z>=a->dimz * 32)
      return;

   a->tiles[z * a->dimx * 32 * a->dimy * 32 + y * a->dimx * 32 + x] = tile;
}
//-------------------------------------
