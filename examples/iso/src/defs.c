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
#include "defs.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef enum
{
   MKR_INVALID = 0,                    //
   MKR_MATERIAL_START = 1,             //type: char[16]
   MKR_MATERIAL_END = 2,               //
   MKR_ADJECTIVE = 3,                  //adjective: char[32]
   MKR_DENSITY = 4,                    //density in mg/cm3: u32
   MKR_ITEM_START = 5,                 //type: char[16]
   MKR_ITEM_END = 6,                   //
   MKR_NAME = 7,                       //name: char[32]
   MKR_BODY_START = 8,                 //type: char[16]
   MKR_BODY_END = 9,                   //
   MKR_ENTITY_START = 10,              //type: char[16]
   MKR_ENTITY_END = 11,                //
   MKR_GROUP_START = 12,               //type: char[16]
   MKR_GROUP_END = 13,                 //
   MKR_BODYPART_START = 14,            //
   MKR_BODYPART_END = 15,              //
   MKR_BODY = 16,                      //bodytype: char[16]
   MKR_BODYPART_VITAL = 17,            //
}Marker;
//-------------------------------------

//Variables
struct
{
   MaterialDef **arr;
   int count;
   int size_exp;
}material_defs;

struct
{
   ItemDef **arr;
   int count;
   int size_exp;
}item_defs;

struct
{
   BodyDef **arr;
   int count;
   int size_exp;
}body_defs;

struct
{
   EntityDef **arr;
   int count;
   int size_exp;
}entity_defs;

struct
{
   GroupDef **arr;
   int count;
   int size_exp;
}group_defs;
//-------------------------------------

//Function prototypes
static void defs_read_material(RvR_rw *rw, const char *path);
static void defs_read_item(RvR_rw *rw, const char *path);
static void defs_read_body(RvR_rw *rw, const char *path);
static int16_t defs_read_bodypart(RvR_rw *rw, const char *path, BodyDef *body);
static void defs_read_entity(RvR_rw *rw, const char *path);
static void defs_read_group(RvR_rw *rw, const char *path);

static int32_t defs_lookup(uint32_t hash, int exp, uint32_t idx);

static int defs_material_insert(MaterialDef *item);
static void defs_material_grow(void);
static int defs_material_search(const char *type);

static int defs_item_insert(ItemDef *mat);
static void defs_item_grow(void);
static int defs_item_search(const char *type);

static int defs_body_insert(BodyDef *body);
static void defs_body_grow(void);
static int defs_body_search(const char *type);

static int defs_entity_insert(EntityDef *ent);
static void defs_entity_grow(void);
static int defs_entity_search(const char *type);

static int defs_group_insert(GroupDef *group);
static void defs_group_grow(void);
static int defs_group_search(const char *type);
//-------------------------------------

//Function implementations

void defs_init(void)
{
   material_defs.size_exp = 8;
   material_defs.count = 0;
   material_defs.arr = RvR_malloc(sizeof(*material_defs.arr)*(1<<material_defs.size_exp),"MaterialDefs hashmap");
   memset(material_defs.arr,0,sizeof(*material_defs.arr)*(1<<material_defs.size_exp));

   item_defs.size_exp = 8;
   item_defs.count = 0;
   item_defs.arr = RvR_malloc(sizeof(*item_defs.arr)*(1<<item_defs.size_exp),"ItemDefs hashmap");
   memset(item_defs.arr,0,sizeof(*item_defs.arr)*(1<<item_defs.size_exp));

   body_defs.size_exp = 8;
   body_defs.count = 0;
   body_defs.arr = RvR_malloc(sizeof(*body_defs.arr)*(1<<body_defs.size_exp),"BodyDefs hashmap");
   memset(body_defs.arr,0,sizeof(*body_defs.arr)*(1<<body_defs.size_exp));

   entity_defs.size_exp = 8;
   entity_defs.count = 0;
   entity_defs.arr = RvR_malloc(sizeof(*entity_defs.arr)*(1<<entity_defs.size_exp),"EntityDefs hashmap");
   memset(entity_defs.arr,0,sizeof(*entity_defs.arr)*(1<<entity_defs.size_exp));

   group_defs.size_exp = 8;
   group_defs.count = 0;
   group_defs.arr = RvR_malloc(sizeof(*group_defs.arr)*(1<<group_defs.size_exp),"GroupDefs hashmap");
   memset(group_defs.arr,0,sizeof(*group_defs.arr)*(1<<group_defs.size_exp));
}

void defs_load(const char *path)
{
   RvR_rw rw = {0};
   RvR_rw_init_path(&rw,path,"rb");

   uint32_t marker = RvR_rw_read_u32(&rw);
   while(!RvR_rw_eof(&rw))
   {
      switch(marker)
      {
      case MKR_MATERIAL_START:
         defs_read_material(&rw,path);
         break;
      case MKR_ITEM_START:
         defs_read_item(&rw,path);
         break;
      case MKR_BODY_START:
         defs_read_body(&rw,path);
         break;
      case MKR_ENTITY_START:
         defs_read_entity(&rw,path);
         break;
      case MKR_GROUP_START:
         defs_read_group(&rw,path);
         break;
      default:
         RvR_log_line("defs_load","invalid marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         break;
      }

      marker = RvR_rw_read_u32(&rw);
   }

   RvR_rw_close(&rw);
}

const MaterialDef *defs_get_material(const char *type)
{
   int idx = defs_material_search(type);
   if(idx<0)
      return NULL;
   return material_defs.arr[idx];
}

const ItemDef *defs_get_item(const char *type)
{
   int idx = defs_item_search(type);
   if(idx<0)
      return NULL;
   return item_defs.arr[idx];
}

const BodyDef *defs_get_body(const char *type)
{
   int idx = defs_body_search(type);
   if(idx<0)
      return NULL;
   return body_defs.arr[idx];
}

const EntityDef *defs_get_entity(const char *type)
{
   int idx = defs_entity_search(type);
   if(idx<0)
      return NULL;
   return entity_defs.arr[idx];
}

const GroupDef *defs_get_group(const char *type)
{
   int idx = defs_group_search(type);
   if(idx<0)
      return NULL;
   return group_defs.arr[idx];
}

static void defs_read_material(RvR_rw *rw, const char *path)
{
   MaterialDef *mat = RvR_malloc(sizeof(*mat),"MaterialDef struct");
   memset(mat,0,sizeof(*mat));

   //Name is always first
   for(int i = 0;i<16;i++) mat->type[i] = RvR_rw_read_u8(rw);
   mat->type[15] = '\0';

   uint32_t marker = RvR_rw_read_u32(rw);
   while(marker!=MKR_MATERIAL_END)
   {
      switch(marker)
      {
      case MKR_ADJECTIVE:
         for(int i = 0;i<32;i++) mat->adjective[i] = RvR_rw_read_u8(rw);
         mat->adjective[31] = '\0';
         break;
      case MKR_DENSITY:
         mat->density = RvR_rw_read_u32(rw);
         break;
      default:
         RvR_log_line("defs_load","invalid material marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         return;
      }

      if(RvR_rw_eof(rw))
      {
         RvR_log_line("defs_load","unterminated material at eof in file '%s'\n",path);
         exit(0);
      }
      marker = RvR_rw_read_u32(rw);
   }

   defs_material_insert(mat);
}

static void defs_read_item(RvR_rw *rw, const char *path)
{
   ItemDef *item = RvR_malloc(sizeof(*item),"ItemDef struct");
   memset(item,0,sizeof(*item));

   //Name is always first
   for(int i = 0;i<16;i++) item->type[i] = RvR_rw_read_u8(rw);
   item->type[15] = '\0';

   uint32_t marker = RvR_rw_read_u32(rw);
   while(marker!=MKR_ITEM_END)
   {
      switch(marker)
      {
      case MKR_NAME:
         for(int i = 0;i<32;i++) item->name[i] = RvR_rw_read_u8(rw);
         item->name[31] = '\0';
         break;
      default:
         RvR_log_line("defs_load","invalid item marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         return;
      }

      if(RvR_rw_eof(rw))
      {
         RvR_log_line("defs_load","unterminated item at eof in file '%s'\n",path);
         exit(0);
      }
      marker = RvR_rw_read_u32(rw);
   }

   defs_item_insert(item);
}

static void defs_read_body(RvR_rw *rw, const char *path)
{
   BodyDef *body = RvR_malloc(sizeof(*body),"BodyDef struct");
   memset(body,0,sizeof(*body));

   //Name is always first
   for(int i = 0;i<16;i++) body->type[i] = RvR_rw_read_u8(rw);
   body->type[15] = '\0';

   uint32_t marker = RvR_rw_read_u32(rw);
   while(marker!=MKR_BODY_END)
   {
      switch(marker)
      {
      //case MKR_NAME:
         //for(int i = 0;i<32;i++) body->name[i] = RvR_rw_read_u8(rw);
         //body->name[31] = '\0';
         //break;
      case MKR_BODYPART_START:
         defs_read_bodypart(rw,path,body);
         break;
      default:
         RvR_log_line("defs_load","invalid body marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         return;
      }

      if(RvR_rw_eof(rw))
      {
         RvR_log_line("defs_load","unterminated body at eof in file '%s'\n",path);
         exit(0);
      }
      marker = RvR_rw_read_u32(rw);
   }

   defs_body_insert(body);
}

static int16_t defs_read_bodypart(RvR_rw *rw, const char *path, BodyDef *body)
{
   int16_t cur = body->bodypart_count++;
   body->bodyparts = RvR_realloc(body->bodyparts,sizeof(*body->bodyparts)*body->bodypart_count,"Body bodyparts");
   body->bodyparts[cur].child = -1;
   body->bodyparts[cur].next = -1;

   int16_t prev = -1;
   uint32_t marker = RvR_rw_read_u32(rw);
   while(marker!=MKR_BODYPART_END)
   {
      switch(marker)
      {
      case MKR_NAME:
         for(int i = 0;i<32;i++) body->bodyparts[cur].name[i] = RvR_rw_read_u8(rw);
         body->bodyparts[cur].name[31] = '\0';
         break;
      case MKR_BODYPART_START:
      {
         int16_t child = defs_read_bodypart(rw,path,body);
         if(prev==-1)
         {
            body->bodyparts[cur].child = child;
         }
         else
         {
            body->bodyparts[prev].next = child;
         }
         prev = child;
      }
         break;
      case MKR_BODYPART_VITAL:
         body->bodyparts[cur].tags|=DEF_BODY_VITAL;
         break;
      default:
         RvR_log_line("defs_load","invalid bodypart marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         return -1;
      }
      marker = RvR_rw_read_u32(rw);
   }

   return cur;
}

static void defs_read_entity(RvR_rw *rw, const char *path)
{
   EntityDef *entity = RvR_malloc(sizeof(*entity),"EntityDef struct");
   memset(entity,0,sizeof(*entity));

   //Name is always first
   for(int i = 0;i<16;i++) entity->type[i] = RvR_rw_read_u8(rw);
   entity->type[15] = '\0';

   uint32_t marker = RvR_rw_read_u32(rw);
   while(marker!=MKR_ENTITY_END)
   {
      switch(marker)
      {
      case MKR_BODY:
      {
         char name[16];
         for(int i = 0;i<16;i++) name[i] = RvR_rw_read_u8(rw);
         name[15] = '\0';
         entity->body = defs_get_body(name);
      }
         break;
      default:
         RvR_log_line("defs_load","invalid entity marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         return;
      }

      if(RvR_rw_eof(rw))
      {
         RvR_log_line("defs_load","unterminated entity at eof in file '%s'\n",path);
         exit(0);
      }
      marker = RvR_rw_read_u32(rw);
   }

   defs_entity_insert(entity);
}

static void defs_read_group(RvR_rw *rw, const char *path)
{
   GroupDef *group = RvR_malloc(sizeof(*group),"GroupDef struct");
   memset(group,0,sizeof(*group));

   //Name is always first
   for(int i = 0;i<16;i++) group->type[i] = RvR_rw_read_u8(rw);
   group->type[15] = '\0';

   uint32_t marker = RvR_rw_read_u32(rw);
   while(marker!=MKR_GROUP_END)
   {
      switch(marker)
      {
      //case MKR_NAME:
         //for(int i = 0;i<32;i++) group->name[i] = RvR_rw_read_u8(rw);
         //group->name[31] = '\0';
         //break;
      default:
         RvR_log_line("defs_load","invalid group marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         return;
      }

      if(RvR_rw_eof(rw))
      {
         RvR_log_line("defs_load","unterminated group at eof in file '%s'\n",path);
         exit(0);
      }
      marker = RvR_rw_read_u32(rw);
   }

   defs_group_insert(group);
}

static int32_t defs_lookup(uint32_t hash, int exp, uint32_t idx)
{
   uint32_t mask = ((uint32_t)1<<exp)-1;
   uint32_t step = (hash>>(32-exp))|1;
   return (idx+step)&mask;
}

static int defs_material_insert(MaterialDef *mat)
{
   uint32_t hash = RvR_fnv32a(mat->type);
   int32_t current = defs_lookup(hash,material_defs.size_exp,hash);
   while(material_defs.arr[current]!=NULL)
   {
      current = defs_lookup(hash,material_defs.size_exp,current);
   }

   material_defs.arr[current] = mat;
   material_defs.count++;

   if(material_defs.count>(1<<(material_defs.size_exp))/2)
      defs_material_grow();
   else
      return current;

   return defs_material_search(mat->type);
}

static void defs_material_grow(void)
{
   int os = 1<<material_defs.size_exp;
   material_defs.size_exp++;
   MaterialDef **old_arr = material_defs.arr;
   material_defs.arr = RvR_malloc(sizeof(*material_defs.arr)*(1<<material_defs.size_exp),"MaterialDefs hashmap");
   memset(material_defs.arr,0,sizeof(*material_defs.arr)*(1<<material_defs.size_exp));
   material_defs.count = 0;

   for(int i = 0;i<os;i++)
   {
      if(old_arr[i]!=NULL)
         defs_material_insert(old_arr[i]);
   }

   RvR_free(old_arr);
}

static int defs_material_search(const char *type)
{
   uint32_t hash = RvR_fnv32a(type);
   int32_t current = defs_lookup(hash,material_defs.size_exp,hash);
   while(material_defs.arr[current]!=NULL)
   {
      if(strcmp(material_defs.arr[current]->type,type)==0)
         return current;
      current = defs_lookup(hash,material_defs.size_exp,current);
   }

   return -1;
}

static int defs_item_insert(ItemDef *item)
{
   uint32_t hash = RvR_fnv32a(item->type);
   int32_t current = defs_lookup(hash,item_defs.size_exp,hash);
   while(item_defs.arr[current]!=NULL)
   {
      current = defs_lookup(hash,item_defs.size_exp,current);
   }

   item_defs.arr[current] = item;
   item_defs.count++;

   if(item_defs.count>(1<<(item_defs.size_exp))/2)
      defs_item_grow();
   else
      return current;

   return defs_item_search(item->type);
}

static void defs_item_grow(void)
{
   int os = 1<<item_defs.size_exp;
   item_defs.size_exp++;
   ItemDef **old_arr = item_defs.arr;
   item_defs.arr = RvR_malloc(sizeof(*item_defs.arr)*(1<<item_defs.size_exp),"ItemDefs hashmap");
   memset(item_defs.arr,0,sizeof(*item_defs.arr)*(1<<item_defs.size_exp));
   item_defs.count = 0;

   for(int i = 0;i<os;i++)
   {
      if(old_arr[i]!=NULL)
         defs_item_insert(old_arr[i]);
   }

   RvR_free(old_arr);
}

static int defs_item_search(const char *type)
{
   uint32_t hash = RvR_fnv32a(type);
   int32_t current = defs_lookup(hash,item_defs.size_exp,hash);
   while(item_defs.arr[current]!=NULL)
   {
      if(strcmp(item_defs.arr[current]->type,type)==0)
         return current;
      current = defs_lookup(hash,item_defs.size_exp,current);
   }

   return -1;
}

static int defs_body_insert(BodyDef *body)
{
   uint32_t hash = RvR_fnv32a(body->type);
   int32_t current = defs_lookup(hash,body_defs.size_exp,hash);
   while(body_defs.arr[current]!=NULL)
   {
      current = defs_lookup(hash,body_defs.size_exp,current);
   }

   body_defs.arr[current] = body;
   body_defs.count++;

   if(body_defs.count>(1<<(body_defs.size_exp))/2)
      defs_body_grow();
   else
      return current;

   return defs_body_search(body->type);
}

static void defs_body_grow(void)
{
   int os = 1<<body_defs.size_exp;
   body_defs.size_exp++;
   BodyDef **old_arr = body_defs.arr;
   body_defs.arr = RvR_malloc(sizeof(*body_defs.arr)*(1<<body_defs.size_exp),"BodyDefs hashmap");
   memset(body_defs.arr,0,sizeof(*body_defs.arr)*(1<<body_defs.size_exp));
   body_defs.count = 0;

   for(int i = 0;i<os;i++)
   {
      if(old_arr[i]!=NULL)
         defs_body_insert(old_arr[i]);
   }

   RvR_free(old_arr);
}

static int defs_body_search(const char *type)
{
   uint32_t hash = RvR_fnv32a(type);
   int32_t current = defs_lookup(hash,body_defs.size_exp,hash);
   while(body_defs.arr[current]!=NULL)
   {
      if(strcmp(body_defs.arr[current]->type,type)==0)
         return current;
      current = defs_lookup(hash,body_defs.size_exp,current);
   }

   return -1;
}

static int defs_entity_insert(EntityDef *ent)
{
   uint32_t hash = RvR_fnv32a(ent->type);
   int32_t current = defs_lookup(hash,entity_defs.size_exp,hash);
   while(entity_defs.arr[current]!=NULL)
   {
      current = defs_lookup(hash,entity_defs.size_exp,current);
   }

   entity_defs.arr[current] = ent;
   entity_defs.count++;

   if(entity_defs.count>(1<<(entity_defs.size_exp))/2)
      defs_entity_grow();
   else
      return current;

   return defs_entity_search(ent->type);
}

static void defs_entity_grow(void)
{
   int os = 1<<entity_defs.size_exp;
   entity_defs.size_exp++;
   EntityDef **old_arr = entity_defs.arr;
   entity_defs.arr = RvR_malloc(sizeof(*entity_defs.arr)*(1<<entity_defs.size_exp),"EntityDefs hashmap");
   memset(entity_defs.arr,0,sizeof(*entity_defs.arr)*(1<<entity_defs.size_exp));
   entity_defs.count = 0;

   for(int i = 0;i<os;i++)
   {
      if(old_arr[i]!=NULL)
         defs_entity_insert(old_arr[i]);
   }

   RvR_free(old_arr);
}

static int defs_entity_search(const char *type)
{
   uint32_t hash = RvR_fnv32a(type);
   int32_t current = defs_lookup(hash,entity_defs.size_exp,hash);
   while(entity_defs.arr[current]!=NULL)
   {
      if(strcmp(entity_defs.arr[current]->type,type)==0)
         return current;
      current = defs_lookup(hash,entity_defs.size_exp,current);
   }

   return -1;
}

static int defs_group_insert(GroupDef *group)
{
   uint32_t hash = RvR_fnv32a(group->type);
   int32_t current = defs_lookup(hash,group_defs.size_exp,hash);
   while(group_defs.arr[current]!=NULL)
   {
      current = defs_lookup(hash,group_defs.size_exp,current);
   }

   group_defs.arr[current] = group;
   group_defs.count++;

   if(group_defs.count>(1<<(group_defs.size_exp))/2)
      defs_group_grow();
   else
      return current;

   return defs_group_search(group->type);
}

static void defs_group_grow(void)
{
   int os = 1<<group_defs.size_exp;
   group_defs.size_exp++;
   GroupDef **old_arr = group_defs.arr;
   group_defs.arr = RvR_malloc(sizeof(*group_defs.arr)*(1<<group_defs.size_exp),"GroupDefs hashmap");
   memset(group_defs.arr,0,sizeof(*group_defs.arr)*(1<<group_defs.size_exp));
   group_defs.count = 0;

   for(int i = 0;i<os;i++)
   {
      if(old_arr[i]!=NULL)
         defs_group_insert(old_arr[i]);
   }

   RvR_free(old_arr);
}

static int defs_group_search(const char *type)
{
   uint32_t hash = RvR_fnv32a(type);
   int32_t current = defs_lookup(hash,group_defs.size_exp,hash);
   while(group_defs.arr[current]!=NULL)
   {
      if(strcmp(group_defs.arr[current]->type,type)==0)
         return current;
      current = defs_lookup(hash,group_defs.size_exp,current);
   }

   return -1;
}
//-------------------------------------
