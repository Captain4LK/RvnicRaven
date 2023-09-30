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
#include "material.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef enum
{
   MKR_MATERIAL_START = 0,             //name:char[16]
   MKR_MATERIAL_END = 1,               //
   MKR_ADJECTIVE = 2,                  //adjective:char[32]
   MKR_DENSITY = 3,                    //density in mg/cm3: u32
}Marker;
//-------------------------------------

//Variables
struct
{
   MaterialDef **arr;
   int count;
   int size_exp;
}material_defs;
//-------------------------------------

//Function prototypes
static void defs_read_material(RvR_rw *rw, const char *path);

static int32_t defs_lookup(uint32_t hash, int exp, uint32_t idx);

static int defs_material_insert(MaterialDef *mat);
static void defs_material_grow(void);
static int defs_material_search(const char *name);
//-------------------------------------

//Function implementations

void defs_init(void)
{
   material_defs.size_exp = 8;
   material_defs.count = 0;
   material_defs.arr = RvR_malloc(sizeof(*material_defs.arr)*(1<<material_defs.size_exp),"MaterialDefs hashmap");
   memset(material_defs.arr,0,sizeof(*material_defs.arr)*(1<<material_defs.size_exp));
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
      defualt:
         RvR_log_line("defs_load","invalid marker %" PRIu32 " in file '%s'\n",marker,path);
         exit(0);
         break;
      }

      marker = RvR_rw_read_u32(&rw);
   }

   RvR_rw_close(&rw);
}

const MaterialDef *defs_get_material(const char *name)
{
   int idx = defs_material_search(name);
   if(idx<0)
      return NULL;
   return material_defs.arr[idx];
}

static void defs_read_material(RvR_rw *rw, const char *path)
{
   MaterialDef *mat = RvR_malloc(sizeof(*mat),"MaterialDef struct");
   memset(mat,0,sizeof(*mat));

   //Name is always first
   for(int i = 0;i<16;i++) mat->name[i] = RvR_rw_read_u8(rw);
   mat->name[15] = '\0';

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

static int32_t defs_lookup(uint32_t hash, int exp, uint32_t idx)
{
   uint32_t mask = ((uint32_t)1<<exp)-1;
   uint32_t step = (hash>>(32-exp))|1;
   return (idx+step)&mask;
}

static int defs_material_insert(MaterialDef *mat)
{
   uint32_t hash = RvR_fnv32a(mat->name);
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

   return defs_material_search(mat->name);
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

static int defs_material_search(const char *name)
{
   uint32_t hash = RvR_fnv32a(name);
   int32_t current = defs_lookup(hash,material_defs.size_exp,hash);
   while(material_defs.arr[current]!=NULL)
   {
      if(strcmp(material_defs.arr[current]->name,name)==0)
         return current;
      current = defs_lookup(hash,material_defs.size_exp,current);
   }

   return -1;
}
//-------------------------------------
