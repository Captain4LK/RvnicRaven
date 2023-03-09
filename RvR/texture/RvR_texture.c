/*
RvnicRaven - texture managment

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR/RvR_rw.h"
#include "RvR/RvR_malloc.h"
#include "RvR/RvR_pak.h"
#include "RvR/RvR_compress.h"
#include "RvR/RvR_texture.h"
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

typedef struct
{
   int32_t last_access;
   uint16_t tex;
}rvr_texture_cache_entry;

static RvR_texture **rvr_textures = NULL;
static int16_t *rvr_textures_cache = NULL;

static int32_t rvr_texture_last_access = 1;
static struct
{
   rvr_texture_cache_entry *cache;
   int cache_used;
} rvr_texture_cache = {.cache = NULL, .cache_used = 0};

static void rvr_texture_load(uint16_t id);

RvR_texture *RvR_texture_get(uint16_t id)
{
   if(rvr_textures==NULL||rvr_textures[id]==NULL)
      rvr_texture_load(id);

   int cache_id = rvr_textures_cache[id];
   if(cache_id==-1)
      return rvr_textures[id];

   rvr_texture_cache.cache[cache_id].last_access = rvr_texture_last_access++;
   if(rvr_texture_last_access==INT32_MAX)
   {
#if RVR_TEXTURE_DEBUG
      RvR_log("normalizing cache\n");
#endif

      rvr_texture_last_access = 0;
      for(int i = 0; i<rvr_texture_cache.cache_used; i++)
      {
         rvr_texture_cache.cache[i].last_access >>= 16;
         if(rvr_texture_cache.cache[i].last_access>rvr_texture_last_access)
            rvr_texture_last_access = rvr_texture_cache.cache[i].last_access;
      }
   }

   return rvr_textures[id];
}

static void rvr_texture_load(uint16_t id)
{
   if(rvr_texture_cache.cache==NULL)
   {
      rvr_texture_cache.cache_used = 0;
      rvr_texture_cache.cache = RvR_malloc(sizeof(*rvr_texture_cache.cache) * RVR_TEXTURE_MAX, "RvR texture cache array");
      memset(rvr_texture_cache.cache, 0, sizeof(*rvr_texture_cache.cache) * RVR_TEXTURE_MAX);
   }
   if(rvr_textures==NULL)
   {
      rvr_textures = RvR_malloc(sizeof(*rvr_textures) * (UINT16_MAX + 1), "RvR texture ptr array");
      rvr_textures_cache = RvR_malloc(sizeof(*rvr_textures_cache) * (UINT16_MAX + 1), "RvR texture cache");
      memset(rvr_textures, 0, sizeof(*rvr_textures) * (UINT16_MAX + 1));
      memset(rvr_textures_cache, 0, sizeof(*rvr_textures_cache) * (UINT16_MAX + 1));
   }

   int index_new = rvr_texture_cache.cache_used;

   //Cache full --> delete 'oldest' texture
   if(rvr_texture_cache.cache_used==RVR_TEXTURE_MAX)
   {
      //Find 'oldest' texture
      int32_t tex_old = rvr_texture_last_access;
      int tex_old_index = 0;
      for(int i = 0; i<rvr_texture_cache.cache_used; i++)
      {
         if(rvr_texture_cache.cache[i].last_access<tex_old)
         {
            tex_old_index = i;
            tex_old = rvr_texture_cache.cache[i].last_access;
         }
      }

#if RVR_TEXTURE_DEBUG
      RvR_log("unloading texture %d\n", rvr_texture_cache.cache[tex_old_index].tex);
#endif

      //Delete texture
      int tex_index = rvr_texture_cache.cache[tex_old_index].tex;
      if(rvr_textures[tex_index]!=NULL)
         RvR_free(rvr_textures[tex_index]);
      rvr_textures[tex_index] = NULL;


      index_new = tex_old_index;
   }
   else
   {
      rvr_texture_cache.cache_used++;
   }

#if RVR_TEXTURE_DEBUG
   RvR_log("loading texture %d\n", id);
#endif

   //Format lump name
   //Textures must be named in this exact way (e.g. TEX00000)
   char tmp[64];
   sprintf(tmp, "TEX%05d", id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   mem_pak = RvR_lump_get(tmp, &size_in);
   RvR_rw rw_decomp;
   RvR_rw_init_const_mem(&rw_decomp, mem_pak, size_in);
   mem_decomp = RvR_crush_decompress(&rw_decomp, &size_out);
   RvR_rw_close(&rw_decomp);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw, mem_decomp, size_out);

   int32_t width = RvR_rw_read_u32(&rw);
   int32_t height = RvR_rw_read_u32(&rw);
   rvr_textures[id] = RvR_malloc(sizeof(*rvr_textures[id]) + sizeof(*rvr_textures[id]->data) * width * height, "RvR texture");
   rvr_textures[id]->width = width;
   rvr_textures[id]->height = height;
   for(int i = 0; i<rvr_textures[id]->width * rvr_textures[id]->height; i++)
      rvr_textures[id]->data[i] = RvR_rw_read_u8(&rw);
   rvr_textures_cache[id] = index_new;
   rvr_texture_cache.cache[index_new].tex = id;

   RvR_rw_close(&rw);

   RvR_free(mem_pak);
   RvR_free(mem_decomp);
}

void RvR_texture_create(uint16_t id, int width, int height)
{
   if(rvr_textures==NULL)
   {
      rvr_textures = RvR_malloc(sizeof(*rvr_textures) * (UINT16_MAX + 1), "RvR texture cache");
      rvr_textures_cache = RvR_malloc(sizeof(*rvr_textures_cache) * (UINT16_MAX + 1), "RvR texture cache array");
      memset(rvr_textures, 0, sizeof(*rvr_textures) * (UINT16_MAX + 1));
      memset(rvr_textures_cache, 0, sizeof(*rvr_textures_cache) * (UINT16_MAX + 1));
   }

   if(rvr_textures[id]!=NULL)
      return;

   rvr_textures[id] = RvR_malloc(sizeof(*rvr_textures[id]) + sizeof(*rvr_textures[id]->data) * width * height, "RvR texture");
   rvr_textures[id]->width = width;
   rvr_textures[id]->height = height;
   rvr_textures_cache[id] = -1;
}

void RvR_texture_create_free(uint16_t id)
{
   RvR_free(rvr_textures[id]);
   rvr_textures[id] = NULL;
}
//-------------------------------------
