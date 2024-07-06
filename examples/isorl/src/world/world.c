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
#include "config.h"
#include "util.h"
#include "world.h"
#include "region.h"
#include "entity_documented.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void world_load_base_file(World *world);
static void world_save_base_file(const World *world);
//-------------------------------------

//Function implementations

World *world_new(const char *name, World_size size)
{
   World *w = NULL;

   RvR_error_check(name!=NULL, "world_new", "world name is null\n");

   w = RvR_malloc(sizeof(*w), "World struct");
   memset(w, 0, sizeof(*w));

   int res = util_mkdir("saves");
   RvR_error_check(res>=0, "world_new", "failed to create directory \"saves\"\n");
   res = snprintf(w->base_path, UTIL_PATH_MAX, "./saves/%s", name);
   RvR_error_check(res<UTIL_PATH_MAX, "world_new", "world base path truncated, path too long\n");
   res = util_mkdir(w->base_path);
   RvR_error_check(res>=0, "world_new", "failed to create directory \"%s\"\n", w->base_path);

   w->size = size;
   int dim = world_size_to_dim(size);
   w->regions = RvR_malloc(sizeof(*w->regions) * dim * dim, "World regions");
   w->region_map = RvR_malloc(sizeof(*w->region_map) * dim * dim, "World region map");
   memset(w->regions, 0, sizeof(*w->regions) * dim * dim);
   memset(w->region_map, 0, sizeof(*w->region_map) * dim * dim);
   w->next_eid = INT64_MAX;
   w->next_iid = INT64_MAX;

   //Create files
   //-------------------------------------
   region_file_create(w);
   world_save_base_file(w);
   //-------------------------------------

   entity_doc_init_table(w);

   return w;

RvR_err:

   if(w!=NULL)
   {
      if(w->regions!=NULL)
         RvR_free(w->regions);
      if(w->region_map!=NULL)
         RvR_free(w->region_map);

      RvR_free(w);
   }

   return NULL;
}

World *world_load(const char *name)
{
   World *w = NULL;

   RvR_error_check(name!=NULL, "world_load", "world name is null\n");

   w = RvR_malloc(sizeof(*w), "World struct");
   memset(w, 0, sizeof(*w));
   int res = snprintf(w->base_path, UTIL_PATH_MAX, "./saves/%s", name);
   RvR_error_check(res<UTIL_PATH_MAX, "world_load", "world base path truncated, path too long\n");

   world_load_base_file(w);
   region_file_load(w);

   int dim = world_size_to_dim(w->size);
   w->regions = RvR_malloc(sizeof(*w->regions) * dim * dim, "World regions");
   w->region_map = RvR_malloc(sizeof(*w->region_map) * dim * dim, "World region map");
   memset(w->regions, 0, sizeof(*w->regions) * dim * dim);
   memset(w->region_map, 0, sizeof(*w->region_map) * dim * dim);

   entity_doc_init_table(w);

   return w;

RvR_err:

   if(w!=NULL)
   {
      if(w->regions!=NULL)
         RvR_free(w->regions);
      if(w->region_map!=NULL)
         RvR_free(w->region_map);

      RvR_free(w);
   }

   return NULL;
}

void world_save(const World *w)
{
   if(w==NULL)
      return;



   world_save_base_file(w);

   //TODO(Captain4LK): save other stuff (region file, etc.)
}

void world_free(World *w)
{
   if(w==NULL)
      return;

   RvR_free(w->regions);
   RvR_free(w->region_map);
   RvR_free(w);
}

unsigned world_size_to_dim(World_size size)
{
   switch(size)
   {
   case WORLD_SMALL:
      return 16;
   case WORLD_MEDIUM:
      return 32;
   case WORLD_LARGE:
      return 64;
   }

   return 0;
}

int32_t world_elevation(World *w, int32_t x, int32_t y)
{
   unsigned dim = world_size_to_dim(w->size);

   if(x<0)
      x = 0;
   if(y<0)
      y = 0;

   int tx = x % 32;
   int ty = y % 32;

   if(x>=dim * 32)
   {
      x = dim * 32 - 1;
      tx = 32;
   }
   if(y>=dim * 32)
   {
      y = dim * 32 - 1;
      ty = 32;
   }

   Region *r = region_get(w, x / 32, y / 32);
   if(r==NULL)
      return 0;

   return r->srf.top[ty * 33 + tx];
}

static void world_load_base_file(World *world)
{
   RvR_rw rw = {0};

   RvR_error_check(world!=NULL, "world_load_base_file", "world is null\n");

   char path[UTIL_PATH_MAX];
   int res = snprintf(path, UTIL_PATH_MAX, "%s/world.bin", world->base_path);
   RvR_error_check(res<UTIL_PATH_MAX, "world_load_base_file", "world base file path truncated, path too long\n");

   RvR_rw_init_path(&rw, path, "rb");

   uint32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0, "world_load_base_file", "version mismatch, expected version 0, got version %d\n", version);

   world->size = RvR_rw_read_u32(&rw);
   RvR_error_check(world->size>=0&&world->size<=2, "world_load_base_file", "invalid world size %d, only 0,1,2 supported\n", world->size);

   //ID counters
   world->next_eid = RvR_rw_read_u64(&rw);
   world->next_deid = RvR_rw_read_u64(&rw);
   world->next_iid = RvR_rw_read_u64(&rw);

   //World preset
   //-------------------------------------
   world->preset.lakes_deep = RvR_rw_read_u32(&rw);
   world->preset.lakes_shallow = RvR_rw_read_u32(&rw);
   world->preset.lakes_rand = RvR_rw_read_u32(&rw);

   world->preset.mountains_high = RvR_rw_read_u32(&rw);
   world->preset.mountains_medium = RvR_rw_read_u32(&rw);

   world->preset.var_elevation = RvR_rw_read_u32(&rw);
   world->preset.var_temperature = RvR_rw_read_u32(&rw);
   world->preset.var_rainfall = RvR_rw_read_u32(&rw);
   //-------------------------------------

   RvR_rw_close(&rw);

RvR_err:

   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   return;
}

static void world_save_base_file(const World *world)
{
   RvR_rw rw = {0};

   RvR_error_check(world!=NULL, "world_save_base_file", "world is null\n");

   char path[UTIL_PATH_MAX];
   int res = snprintf(path, UTIL_PATH_MAX, "%s/world.bin", world->base_path);
   RvR_error_check(res<UTIL_PATH_MAX, "world_save_base_file", "world base file path truncated, path too long\n");

   RvR_rw_init_path(&rw, path, "wb");

   //Version
   RvR_rw_write_u32(&rw, 0);

   //World dimension
   RvR_rw_write_u32(&rw, world->size);

   //ID counters
   RvR_rw_write_u64(&rw, world->next_eid);
   RvR_rw_write_u64(&rw, world->next_deid);
   RvR_rw_write_u64(&rw, world->next_iid);

   //World preset
   //-------------------------------------
   RvR_rw_write_u32(&rw, world->preset.lakes_deep);
   RvR_rw_write_u32(&rw, world->preset.lakes_shallow);
   RvR_rw_write_u32(&rw, world->preset.lakes_rand);

   RvR_rw_write_u32(&rw, world->preset.mountains_high);
   RvR_rw_write_u32(&rw, world->preset.mountains_medium);

   RvR_rw_write_u32(&rw, world->preset.var_elevation);
   RvR_rw_write_u32(&rw, world->preset.var_temperature);
   RvR_rw_write_u32(&rw, world->preset.var_rainfall);
   //-------------------------------------

   RvR_rw_close(&rw);

RvR_err:

   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   return;
}
//-------------------------------------
