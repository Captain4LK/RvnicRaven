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
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "entity_documented.h"
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
static int32_t table_lookup(uint64_t hash, int exp, uint32_t idx);
static uint64_t mix64(uint64_t x);
//-------------------------------------

//Function implementations

void entity_doc_init_table(World *w)
{
   w->doctable.size_exp = 10;
   w->doctable.arr = RvR_malloc(sizeof(*w->doctable.arr)*(1<<w->doctable.size_exp),"World doctable array");
   w->doctable.count = 0;

   for(int i = 0;i<1<<w->doctable.size_exp;i++)
      w->doctable.arr[i] = NULL;
}

void entity_doc_get(World *w, Entity_documented *e, uint64_t id)
{
   if(e==NULL)
      return;

   //Not a DocEnt id
   if(id&(UINT64_C(1)<<63))
      return;

   //Search in table
   int32_t idx = table_search(w,id);
   if(idx>=0)
   {
      *e = *w->doctable.arr[idx];
      return;
   }

   //Not found --> try to load and insert
   Entity_documented *ne = entity_doc_load(w,id);
   if(ne!=NULL)
   {
      table_insert(w,id,ne);
      *e = *ne;
      return;
   }

   //Doesn't exist --> create new, empty entity
   ne = RvR_malloc(sizeof(*ne),"Entity_documented struct");
   memset(ne,0,sizeof(*ne));
   ne->id = id;
   ne->modified = 1;
   table_insert(w,id,ne);
   *e = *ne;
}

void entity_doc_modify(World *w, uint64_t id, const Entity_documented *e)
{
   int32_t idx = table_search(w,id);
   if(idx<0)
      return;


   Entity_documented *te = w->doctable.arr[idx];
   uint64_t te_id = te->id;

   memcpy(te,e,sizeof(*e));
   te->modified = 1;
   te->id = te_id;
}

void entity_doc_save_modified(World *w)
{
   for(int i = 0;i<1<<w->doctable.size_exp;i++)
   {
      if(w->doctable.arr[i]!=NULL&&w->doctable.arr[i]->modified)
         entity_doc_save(w,w->doctable.arr[i]->id);
   }
}

static Entity_documented *entity_doc_load(World *w, uint64_t id)
{
   Entity_documented *e = NULL;
   RvR_rw rw = {0};
   RvR_rw rw_decomp = {0};
   RvR_rw rw_reg = {0};
   uint8_t *mem_decomp = NULL;

   int file_id = id/256;
   char path[UTIL_PATH_MAX];
   int res = snprintf(path,UTIL_PATH_MAX,"%s/entities%05d.dat",w->base_path,file_id);
   RvR_error_check(res<UTIL_PATH_MAX, "entity_doc_load", "entity file name truncated, path too long\n");
   RvR_rw_init_path(&rw,path,"rb");
   RvR_error_check(RvR_rw_valid(&rw),"entity_doc_load","failed to open file '%s'\n",path);

   int32_t version = RvR_rw_read_u32(&rw);
   RvR_error_check(version==0,"entity_doc_load","version mismatch, expected version 0, got version %d\n",version);
   RvR_rw_seek(&rw,8+(file_id*256-id)*4,SEEK_SET);
   int32_t offset = RvR_rw_read_u32(&rw);
   RvR_error_check(offset>=0,"entity_doc_load","offset is negative, entity not created yet\n");

   RvR_rw_seek(&rw,offset,SEEK_SET);
   int32_t size = RvR_rw_read_u32(&rw);
   RvR_error_check(size>=0,"entity_doc_load","size of entity %u is negative, file might be corrupt\n",id);

   if(entity_doc_buffer==NULL||entity_doc_buffer_size<size)
   {
      entity_doc_buffer = RvR_realloc(entity_doc_buffer,size,"entity_documented buffer");
      entity_doc_buffer_size = size;
   }

   RvR_mem_tag_set(entity_doc_buffer,RVR_MALLOC_STATIC);
   RvR_mem_usr_set(entity_doc_buffer,(void **)&entity_doc_buffer);
   RvR_rw_read(&rw,entity_doc_buffer,1,size);
   RvR_rw_close(&rw);

   RvR_rw_init_const_mem(&rw_decomp,entity_doc_buffer,size);
   int32_t size_out = 0;
   mem_decomp = RvR_crush_decompress(&rw_decomp,&size_out);
   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);

   RvR_rw_init_const_mem(&rw_reg,mem_decomp,size_out);

   //Create entity from data
   //-------------------------------------
   e = RvR_malloc(sizeof(*e),"DocEnt struct"); 
   //-------------------------------------

   RvR_rw_close(&rw_reg);
   //Changing tags to cache needs to be done last!
   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_CACHE);
   RvR_mem_tag_set(entity_doc_buffer,RVR_MALLOC_CACHE);

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
      RvR_mem_tag_set(entity_doc_buffer,RVR_MALLOC_CACHE);

   if(mem_decomp!=NULL)
      RvR_mem_tag_set(mem_decomp,RVR_MALLOC_CACHE);

   return NULL;
}

static void entity_doc_save(World *w, uint64_t id)
{
}

static int32_t table_insert(World *w, uint64_t id, Entity_documented *e)
{
   uint64_t hash = mix64(id);
   int32_t current = table_lookup(hash,w->doctable.size_exp,hash);
   while(w->doctable.arr[current]!=NULL)
   {
      if(w->doctable.arr[current]->id==id)
         return current;
      current = table_lookup(hash,w->doctable.size_exp,current);
   }

   e->id = id;
   w->doctable.arr[current] = e;
   w->doctable.count++;

   if(w->doctable.count>(1<<w->doctable.size_exp)/2)
      table_grow(w);
   else
      return current;

   return table_search(w,id);
}

static int32_t table_search(World *w, uint64_t id)
{
   uint64_t hash = mix64(id);
   int32_t current = table_lookup(hash,w->doctable.size_exp,hash);
   while(w->doctable.arr[current]!=NULL)
   {
      if(w->doctable.arr[current]->id==id)
         return current;

      current = table_lookup(hash,w->doctable.size_exp,current);
   }

   return -1;
}

static void table_grow(World *w)
{
   size_t os = 1<<w->doctable.size_exp;
   w->doctable.size_exp++;

   Entity_documented **old_arr = w->doctable.arr;
   w->doctable.arr = RvR_malloc(sizeof(*w->doctable.arr)*(1<<w->doctable.size_exp),"World doctable arr");
   for(int i = 0;i<1<<w->doctable.size_exp;i++)
      w->doctable.arr[i] = NULL;
   w->doctable.count = 0;

   for(int i = 0;i<os;i++)
   {
      if(old_arr[i]!=NULL)
         table_insert(w,old_arr[i]->id,old_arr[i]);
   }
}

static int32_t table_lookup(uint64_t hash, int exp, uint32_t idx)
{
   uint32_t mask = ((uint32_t)1<<exp)-1;
   uint32_t step = (hash>(64-exp))|1;
   return (idx+step)&mask;
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
