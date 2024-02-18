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
#include "RvR/RvR_log.h"
#include "RvR/RvR_app.h"
#include "RvR/RvR_rw.h"
#include "RvR/RvR_malloc.h"
#include "RvR/RvR_pak.h"
#include "RvR/RvR_compress.h"
#include "RvR/RvR_texture.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
static RvR_texture **rvr_textures = NULL;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void rvr_texture_load(uint16_t id);
//-------------------------------------

//Function implementations

RvR_texture *RvR_texture_get(uint16_t id)
{
   if(rvr_textures==NULL||rvr_textures[id]==NULL)
      rvr_texture_load(id);

   //Animated texture
   if(rvr_textures[id]->anim_range!=0)
   {
      uint16_t tex = (uint16_t)(id + (RvR_frame() / rvr_textures[id]->anim_speed) % rvr_textures[id]->anim_range);
      if(rvr_textures[tex]==NULL)
         rvr_texture_load(tex);
      return rvr_textures[tex];
   }

   return rvr_textures[id];
}

static void rvr_texture_load(uint16_t id)
{
   if(rvr_textures==NULL)
   {
      rvr_textures = RvR_malloc(sizeof(*rvr_textures) * (UINT16_MAX + 1), "RvR texture ptr array");
      memset(rvr_textures, 0, sizeof(*rvr_textures) * (UINT16_MAX + 1));
   }

   //Format lump name
   //Textures must be named in this exact way (e.g. TEX00000)
   char tmp[64];
   sprintf(tmp, "TEX%05d", id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak = NULL;
   uint8_t *mem_decomp = NULL;
   mem_pak = RvR_lump_get(tmp, &size_in);

   RvR_error_check(size_in!=0, "RvR_texture_get", "TEX%05d not found\n", id);

   RvR_mem_tag_set(mem_pak, RVR_MALLOC_STATIC);
   RvR_rw rw_decomp = {0};
   RvR_rw_init_const_mem(&rw_decomp, mem_pak, size_in);
   mem_decomp = RvR_crush_decompress(&rw_decomp, &size_out);
   RvR_mem_tag_set(mem_decomp, RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);
   RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   RvR_rw rw = {0};
   RvR_rw_init_const_mem(&rw, mem_decomp, size_out);

   uint16_t version = RvR_rw_read_u16(&rw);
   RvR_error_check(version==0, "RvR_texture_get", "'TEX%05d' has invalid version %d, expected version 0\n", id, version);

   int32_t width = RvR_rw_read_u32(&rw);
   int32_t height = RvR_rw_read_u32(&rw);

   rvr_textures[id] = RvR_malloc(sizeof(*rvr_textures[id]) + sizeof(*rvr_textures[id]->data) * width * height, "RvR texture");
   rvr_textures[id]->width = width;
   rvr_textures[id]->height = height;
   rvr_textures[id]->anim_range = RvR_rw_read_u8(&rw);
   rvr_textures[id]->anim_speed = RvR_rw_read_u8(&rw);
   for(int i = 0; i<rvr_textures[id]->width * rvr_textures[id]->height; i++)
      rvr_textures[id]->data[i] = RvR_rw_read_u8(&rw);
   RvR_mem_tag_set(rvr_textures[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(rvr_textures[id], (void **)&rvr_textures[id]);

   RvR_rw_close(&rw);

   RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);

   return;

RvR_err:

   if(mem_decomp!=NULL)
      RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);
   if(mem_pak!=NULL)
      RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   rvr_textures[id] = RvR_malloc(sizeof(*rvr_textures[id]) + sizeof(*rvr_textures[id]->data), "RvR texture");
   rvr_textures[id]->width = 1;
   rvr_textures[id]->height = 1;
   rvr_textures[id]->anim_range = 0;
   rvr_textures[id]->anim_speed = 0;
   rvr_textures[id]->data[0] = 0;

   RvR_mem_tag_set(rvr_textures[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(rvr_textures[id], (void **)&rvr_textures[id]);
}

void RvR_texture_create(uint16_t id, int width, int height)
{
   if(rvr_textures==NULL)
   {
      rvr_textures = RvR_malloc(sizeof(*rvr_textures) * (UINT16_MAX + 1), "RvR texture cache");
      memset(rvr_textures, 0, sizeof(*rvr_textures) * (UINT16_MAX + 1));
   }

   if(rvr_textures[id]!=NULL)
      return;

   rvr_textures[id] = RvR_malloc(sizeof(*rvr_textures[id]) + sizeof(*rvr_textures[id]->data) * width * height, "RvR texture");
   rvr_textures[id]->width = width;
   rvr_textures[id]->height = height;
   rvr_textures[id]->anim_range = 0;
   rvr_textures[id]->anim_speed = 0;
}

void RvR_texture_create_free(uint16_t id)
{
   RvR_free(rvr_textures[id]);
   rvr_textures[id] = NULL;
}
//-------------------------------------
