/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "entity_documented.h"
#include "entity.h"
#include "area.h"
#include "tile.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t *entity_doc_buffer = NULL;
static int32_t entity_doc_buffer_size = 0;
//-------------------------------------

//Function prototypes
static Entity_documented *entity_doc_load(World *w, uint64_t id);
static void entity_doc_save(World *w, uint64_t id);

static int32_t table_insert(World *w, uint64_t id, Entity_documented *e);
static int32_t table_search(World *w, uint64_t id);
static void table_grow(World *w);
static int32_t table_lookup(uint64_t hash, uint8_t exp, uint64_t idx);
static uint64_t mix64(uint64_t x);
//-------------------------------------

//Function implementations

void entity_doc_init_table(World *w)
{
   w->doctable.size_exp = 10;
   w->doctable.arr = RvR_malloc(sizeof(*w->doctable.arr) * (1 << w->doctable.size_exp), "World doctable array");
   w->doctable.count = 0;

   for(int i = 0; i<1 << w->doctable.size_exp; i++)
      w->doctable.arr[i] = NULL;
}

int entity_doc_get(World *w, uint64_t id, Entity_documented *e)
{
   if(e==NULL)
      return 1;

   //Not a DocEnt id
   if(id & (UINT64_C(1) << 63))
      return 1;

   //Search in table
   int32_t idx = table_search(w, id);
   if(idx>=0)
   {
      if(e!=NULL)
         *e = *w->doctable.arr[idx];
      return 0;
   }

   //Not found --> try to load and insert
   Entity_documented *ne = entity_doc_load(w, id);
   if(ne!=NULL)
   {
      table_insert(w, id, ne);
      if(e!=NULL)
         *e = *ne;
      return 0;
   }

   if(e!=NULL)
      *e = (Entity_documented){
         0
      }
   ;

   return 1;
}

uint64_t entity_doc_create(World *w)
{
   Entity_documented *ne = RvR_malloc(sizeof(*ne), "DocEnt struct");
   memset(ne, 0, sizeof(*ne));
   ne->id = w->next_deid++;
   ne->modified = 1;
   table_insert(w, ne->id, ne);

   return ne->id;
}

void entity_doc_modify(World *w, uint64_t id, const Entity_documented *e)
{
   //Ensures entity is loaded, if it exists
   entity_doc_get(w, id, NULL);

   int32_t idx = table_search(w, id);
   if(idx<0)
      return;

   Entity_documented *te = w->doctable.arr[idx];
   uint64_t te_id = te->id;

   memcpy(te, e, sizeof(*e));
   te->modified = 1;
   te->id = te_id;
}

void entity_doc_save_modified(World *w)
{
   for(int i = 0; i<1 << w->doctable.size_exp; i++)
   {
      if(w->doctable.arr[i]!=NULL&&w->doctable.arr[i]->modified)
      {
         entity_doc_save(w, w->doctable.arr[i]->id);
         w->doctable.arr[i]->modified = 0;
      }
   }
}

int entity_is_docent(const Entity *e)
{
   if(e==NULL)
      return 0;

   return !(e->id & (UINT64_C(1) << 63));
}

Entity *entity_from_docent(World *w, Area *a, uint64_t id)
{
   int32_t idx = table_search(w, id);
   if(idx<0)
      return NULL;

   Entity_documented *de = w->doctable.arr[idx];
   if(de->mx<a->mx||de->my<a->my||de->mx>=a->mx + a->dimx||de->my>=a->my + a->dimy)
      return NULL;

   Entity *e = entity_new(w);
   e->id = id;
   e->x = (int16_t)((de->mx - a->mx) * 32 + de->ax);
   e->y = (int16_t)((de->my - a->my) * 32 + de->ay);
   e->z = (int16_t)(a->dimz * 32 - 1);
   for(int16_t z = 0; z<a->dimz * 32; z++)
   {
      e->z = z;
      if(tile_has_wall(area_tile(a, e->x, e->y, z + 1)))
         break;
   }

   entity_add(a, e);
   entity_grid_add(a, e);

   //TODO(Captain4LK): ai and stats

   return e;
}

void docent_from_entity(World *w, Area *a, Entity *e)
{
   //Not documented
   if(e->id & (UINT64_C(1) << 63))
      return;

   Entity_documented de = {0};
   entity_doc_get(w, e->id, &de);

   de.mx = (uint16_t)((a->mx * 32 + e->x) / 32);
   de.my = (uint16_t)((a->my * 32 + e->y) / 32);
   de.ax = e->x & 31;
   de.ay = e->y & 31;

   entity_doc_modify(w, e->id, &de);
}

static Entity_documented *entity_doc_load(World *w, uint64_t id)
{
   Entity_documented *e = NULL;
   RvR_rw rw = {0};
   RvR_rw rw_decomp = {0};
   RvR_rw rw_reg = {0};
   uint8_t *mem_decomp = NULL;

   uint64_t file_id = id / 256;
   char path[UTIL_PATH_MAX];
   int res = snprintf(path, UTIL_PATH_MAX, "%s/entities%05" PRIu64 ".dat", w->base_path, file_id);
   RvR_error_check(res<UTIL_PATH_MAX, "entity_doc_load", "entity file name truncated, path too long\n");
   RvR_rw_init_path(&rw, path, "rb");
   RvR_error_check(RvR_rw_valid(&rw), "entity_doc_load", "failed to open file '%s'\n", path);

   int32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0, "entity_doc_load", "version mismatch, expected version 0, got version %d\n", version);
   RvR_rw_seek(&rw, 8 + (file_id * 256 - id) * 4, SEEK_SET);
   int32_t offset = RvR_rw_read_u32(&rw);
   RvR_error_check(offset>=0, "entity_doc_load", "offset is negative, entity not created yet\n");

   RvR_rw_seek(&rw, offset, SEEK_SET);
   int32_t size = RvR_rw_read_u32(&rw);
   RvR_error_check(size>=0, "entity_doc_load", "size of entity %u is negative, file might be corrupt\n", id);

   if(entity_doc_buffer==NULL||entity_doc_buffer_size<size)
   {
      entity_doc_buffer = RvR_realloc(entity_doc_buffer, size, "entity_documented buffer");
      entity_doc_buffer_size = size;
   }

   RvR_mem_tag_set(entity_doc_buffer, RVR_MALLOC_STATIC);
   RvR_mem_usr_set(entity_doc_buffer, (void **)&entity_doc_buffer);
   RvR_rw_read(&rw, entity_doc_buffer, 1, size);
   RvR_rw_close(&rw);

   RvR_rw_init_const_mem(&rw_decomp, entity_doc_buffer, size);
   int32_t size_out = 0;
   mem_decomp = RvR_crush_decompress(&rw_decomp, &size_out);
   RvR_mem_tag_set(mem_decomp, RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);

   RvR_rw_init_const_mem(&rw_reg, mem_decomp, size_out);

   //Create entity from data
   //-------------------------------------
   e = RvR_malloc(sizeof(*e), "DocEnt struct");
   e->id = id;
   e->modified = 0;

   e->ident.species = RvR_rw_read_u32(&rw_reg);
   e->ident.gender = RvR_rw_read_u32(&rw_reg);
   e->ident.profession = RvR_rw_read_u32(&rw_reg);

   e->mx = RvR_rw_read_u16(&rw_reg);
   e->my = RvR_rw_read_u16(&rw_reg);
   e->ax = RvR_rw_read_u16(&rw_reg);
   e->ay = RvR_rw_read_u16(&rw_reg);

   RvR_rw_read(&rw, e->name, 64, 1);
   //-------------------------------------

   RvR_rw_close(&rw_reg);
   //Changing tags to cache needs to be done last!
   RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);
   RvR_mem_tag_set(entity_doc_buffer, RVR_MALLOC_CACHE);

   return e;

RvR_err:

   if(e!=NULL)
   {
      RvR_free(e);
   }

   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   if(RvR_rw_valid(&rw_decomp))
      RvR_rw_close(&rw_decomp);

   if(RvR_rw_valid(&rw_reg))
      RvR_rw_close(&rw_reg);

   //Changing tags to cache needs to be done last!
   if(entity_doc_buffer!=NULL)
      RvR_mem_tag_set(entity_doc_buffer, RVR_MALLOC_CACHE);

   if(mem_decomp!=NULL)
      RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);

   return NULL;
}

static void entity_doc_save(World *w, uint64_t id)
{
   RvR_rw rw = {0};
   RvR_rw rw_comp = {0};
   RvR_rw rw_comp_out = {0};

   RvR_error_check(w!=NULL, "entity_doc_save", "world is null\n");

   int32_t idx = table_search(w, id);
   RvR_error_check(idx>=0, "entity_doc_save", "entity %" PRIu64 "not loaded\n", id);

   uint8_t *comp_out = NULL;
   uint64_t file_id = id / 256;
   char path[UTIL_PATH_MAX];
   int res = snprintf(path, UTIL_PATH_MAX, "%s/entities%05" PRIu64 ".dat", w->base_path, file_id);
   RvR_error_check(res<UTIL_PATH_MAX, "entity_doc_save", "entity file name truncated, path too long\n");

   //Read offsets
   RvR_rw_init_path(&rw, path, "rb+");
   if(!RvR_rw_valid(&rw))
   {
      //File doesn't exist
      RvR_rw_init_path(&rw, path, "wb");
      RvR_error_check(RvR_rw_valid(&rw), "entity_doc_save", "failed to create file '%s'\n", path);

      RvR_rw_write_u32(&rw, 0);
      RvR_rw_write_u32(&rw, 1032);
      for(int i = 0; i<256; i++)
         RvR_rw_write_u32(&rw, -1);

      RvR_rw_close(&rw);
      RvR_rw_init_path(&rw, path, "rb+");
   }

   uint32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0, "entity_doc_save", "version mismatch, expected version 0, got version %d\n", version);

   int32_t offset_next = RvR_rw_read_u32(&rw);
   int32_t offsets[256];
   for(int i = 0; i<256; i++)
      offsets[i] = RvR_rw_read_u32(&rw);
   int32_t offset = offsets[file_id * 256 - id];

   //Compress
   //-------------------------------------
   int32_t size = 4 * 3 + 64 + 2 * 4;

   if(entity_doc_buffer==NULL||entity_doc_buffer_size<size)
   {
      entity_doc_buffer = RvR_realloc(entity_doc_buffer, size, "entity_documented buffer");
      entity_doc_buffer_size = size;
   }

   RvR_mem_tag_set(entity_doc_buffer, RVR_MALLOC_STATIC);
   RvR_rw_init_mem(&rw_comp, entity_doc_buffer, size, 0);

   //Write entity
   RvR_rw_write_u32(&rw_comp, w->doctable.arr[idx]->ident.species);
   RvR_rw_write_u32(&rw_comp, w->doctable.arr[idx]->ident.gender);
   RvR_rw_write_u32(&rw_comp, w->doctable.arr[idx]->ident.profession);

   RvR_rw_write_u16(&rw_comp, w->doctable.arr[idx]->mx);
   RvR_rw_write_u16(&rw_comp, w->doctable.arr[idx]->my);
   RvR_rw_write_u16(&rw_comp, w->doctable.arr[idx]->ax);
   RvR_rw_write_u16(&rw_comp, w->doctable.arr[idx]->ay);

   RvR_rw_write(&rw_comp, w->doctable.arr[idx]->name, 64, 1);

   RvR_rw_init_dyn_mem(&rw_comp_out, size, 1);
   RvR_crush_compress(&rw_comp, &rw_comp_out, 10);
   RvR_rw_seek(&rw_comp_out, 0, SEEK_END);
   size = (int32_t)RvR_rw_tell(&rw_comp_out);
   comp_out = rw_comp_out.as.dmem.mem;

   RvR_rw_close(&rw_comp);
   RvR_mem_tag_set(entity_doc_buffer, RVR_MALLOC_CACHE);
   //-------------------------------------

   //Get old size of block
   RvR_rw_seek(&rw, offset, SEEK_SET);
   int32_t size_old = RvR_rw_read_u32(&rw);

   //Adjust indices
   //-------------------------------------
   if(offset==-1)
   {
      offset = offset_next;
      offset_next += size + 4;
      offsets[file_id * 256 - id] = offset;
   }
   else
   {
      for(int i = 0; i<256; i++)
         if(offsets[i]>offset)
            offsets[i] += size - size_old;
      offset_next += size - size_old;
   }

   //Rewrite indices in file
   RvR_rw_seek(&rw, 4, SEEK_SET);
   RvR_rw_write_u32(&rw, offset_next);
   for(int i = 0; i<256; i++)
      RvR_rw_write_u32(&rw, offsets[i]);
   //-------------------------------------

   //Move blocks
   //-------------------------------------
   if(size>size_old)
   {
      //Move data by (size-old_size) bytes
      RvR_rw_seek(&rw, 0, SEEK_END);
      int32_t file_size_old = (int32_t)RvR_rw_tell(&rw);
      int32_t file_size_new = file_size_old + (size - size_old);

      util_truncate(&rw, file_size_new);

      int32_t to_move = file_size_old - offset;
      int32_t pos_read = file_size_old;
      int32_t pos_write = file_size_new;
      for(int i = 0; i<=to_move / 4096; i++)
      {
         int32_t block = RvR_min(4096, pos_read - offset);
         uint8_t buffer[4096];

         //Read
         RvR_rw_seek(&rw, pos_read - block, SEEK_SET);
         RvR_rw_read(&rw, buffer, block, 1);

         //Write
         RvR_rw_seek(&rw, pos_write - block, SEEK_SET);
         RvR_rw_write(&rw, buffer, block, 1);

         pos_read -= block;
         pos_write -= block;
      }
   }
   else if(size<size_old)
   {
      //Move data by (size-old_size) bytes
      //Different to above, since we need to move from left
      //instead of from right
      RvR_rw_seek(&rw, 0, SEEK_END);
      int32_t file_size_old = (int32_t)RvR_rw_tell(&rw);
      int32_t file_size_new = file_size_old + (size - size_old);

      int32_t to_move = file_size_old - offset;
      int32_t pos_read = offset + size_old;
      int32_t pos_write = offset + size;
      for(int i = 0; i<=to_move / 4096; i++)
      {
         int32_t block = RvR_min(4096, file_size_old - pos_read);
         uint8_t buffer[4096];

         //Read
         RvR_rw_seek(&rw, pos_read, SEEK_SET);
         RvR_rw_read(&rw, buffer, block, 1);

         //Write
         RvR_rw_seek(&rw, pos_write, SEEK_SET);
         RvR_rw_write(&rw, buffer, block, 1);

         pos_read += block;
         pos_write += block;
      }

      util_truncate(&rw, file_size_new);
   }
   //-------------------------------------

   //Write new data
   RvR_rw_seek(&rw, offset, SEEK_SET);
   RvR_rw_write_u32(&rw, size);
   RvR_rw_write(&rw, comp_out, 1, size);

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
   if(entity_doc_buffer!=NULL)
      RvR_mem_tag_set(entity_doc_buffer, RVR_MALLOC_CACHE);

   return;
}

static int32_t table_insert(World *w, uint64_t id, Entity_documented *e)
{
   uint64_t hash = mix64(id);
   int32_t current = table_lookup(hash, w->doctable.size_exp, hash);
   while(w->doctable.arr[current]!=NULL)
   {
      if(w->doctable.arr[current]->id==id)
         return current;
      current = table_lookup(hash, w->doctable.size_exp, current);
   }

   e->id = id;
   w->doctable.arr[current] = e;
   w->doctable.count++;

   if(w->doctable.count>(1 << w->doctable.size_exp) / 2)
      table_grow(w);
   else
      return current;

   return table_search(w, id);
}

static int32_t table_search(World *w, uint64_t id)
{
   uint64_t hash = mix64(id);
   int32_t current = table_lookup(hash, w->doctable.size_exp, hash);
   while(w->doctable.arr[current]!=NULL)
   {
      if(w->doctable.arr[current]->id==id)
         return current;

      current = table_lookup(hash, w->doctable.size_exp, current);
   }

   return -1;
}

static void table_grow(World *w)
{
   size_t os = 1 << w->doctable.size_exp;
   w->doctable.size_exp++;

   Entity_documented **old_arr = w->doctable.arr;
   w->doctable.arr = RvR_malloc(sizeof(*w->doctable.arr) * (1 << w->doctable.size_exp), "World doctable arr");
   for(int i = 0; i<1 << w->doctable.size_exp; i++)
      w->doctable.arr[i] = NULL;
   w->doctable.count = 0;

   for(int i = 0; i<os; i++)
   {
      if(old_arr[i]!=NULL)
         table_insert(w, old_arr[i]->id, old_arr[i]);
   }
}

static int32_t table_lookup(uint64_t hash, uint8_t exp, uint64_t idx)
{
   uint64_t mask = (UINT64_C(1) << exp) - 1;
   uint64_t step = (hash>(64 - exp)) | 1;
   return (int32_t)((idx + step) & mask);
}

//https://xoshiro.di.unimi.it/splitmix64.c
/*  Written in 2015 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */
static uint64_t mix64(uint64_t x)
{
   x ^= x >> 30;
   x *= UINT64_C(0xbf58476d1ce4e5b9);
   x ^= x >> 27;
   x *= UINT64_C(0x94d049bb133111eb);
   x ^= x >> 31;
   return x;
}

//-------------------------------------
