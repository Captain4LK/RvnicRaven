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
#include "RvR/RvR_math.h"
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
static RvR_itexture **rvr_textures = NULL;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void rvr_texture_load(uint16_t id);
//-------------------------------------

//Function implementations

RvR_texture *RvR_texture_get(uint16_t id)
{
   RvR_itexture *itex = RvR_itexture_get(id);

   if((itex->flags&RVR_TEXTURE_MULTI)==RVR_TEXTURE_ANIM)
   {
      //TODO(Captain4LK): we could do the same caching here, but we would need to check if we crossed a animation frame boundary
      int offset = (RvR_frame() / itex->anim_speed) % itex->anim_frames;

      itex->tex.width = itex->width;
      itex->tex.height = itex->height;
      itex->tex.exp = 0;
      uint32_t data_per = 0;
      if(itex->flags&RVR_TEXTURE_MIPMAP)
         data_per = itex->width*itex->height+(itex->width*itex->height)/3;
      else
         data_per = itex->width*itex->height;
      itex->tex.data = itex->data+data_per*offset;

      return &itex->tex;
   }
   else
   {
      //Mipmaping doesn't matter here
      itex->tex.width = itex->width;
      itex->tex.height = itex->height;
      itex->tex.exp = 0;
      itex->tex.data = itex->data;
      return &itex->tex;
   }
}

RvR_texture *RvR_texture_get_mipmap(uint16_t id, uint32_t level)
{
   RvR_itexture *itex = RvR_itexture_get(id);
   if(level==0||!(itex->flags&RVR_TEXTURE_MIPMAP)||itex->miplevels==0||RvR_key_down(RVR_KEY_T))
      return RvR_texture_get(id);

   level = RvR_min(level,itex->miplevels);

   if((itex->flags&RVR_TEXTURE_MULTI)==RVR_TEXTURE_ANIM)
   {
      //TODO(Captain4LK): we could do the same caching here, but we would need to check if we crossed a animation frame boundary
      size_t off = (itex->width*itex->height*(int64_t)(1-(((int64_t)1)<<(2*level))))/((((int64_t)1)<<(2*level-2))*-3);
      int offset = (RvR_frame() / itex->anim_speed) % itex->anim_frames;

      itex->tex.width = itex->width/(1<<level);
      itex->tex.height = itex->height/(1<<level);
      itex->tex.exp = level;
      uint32_t data_per = 0;
      if(itex->flags&RVR_TEXTURE_MIPMAP)
         data_per = itex->width*itex->height+(itex->width*itex->height)/3;
      else
         data_per = itex->width*itex->height;
      itex->tex.data = itex->data+data_per*offset+off;

      return &itex->tex;
   }
   else
   {
      if(itex->tex.exp==level)
         return &itex->tex;

      size_t off = (itex->width*itex->height*(int64_t)(1-(((int64_t)1)<<(2*level))))/((((int64_t)1)<<(2*level-2))*-3);
      itex->tex.width = itex->width/(1<<level);
      itex->tex.height = itex->height/(1<<level);
      itex->tex.exp = level;
      itex->tex.data = itex->data+off;
      return &itex->tex;
   }
}

RvR_itexture *RvR_itexture_get(uint16_t id)
{
   if(rvr_textures==NULL||rvr_textures[id]==NULL)
      rvr_texture_load(id);

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

   size_t size_in;
   size_t size_out;
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
   RvR_error_check(version==1, "RvR_texture_get", "'TEX%05d' has invalid version %d, expected version 1\n", id, version);

   int32_t width = RvR_rw_read_u32(&rw);
   int32_t height = RvR_rw_read_u32(&rw);
   uint32_t flags = RvR_rw_read_u32(&rw);
   uint8_t anim_speed = RvR_rw_read_u8(&rw);
   uint8_t anim_frames = RvR_rw_read_u8(&rw);

   size_t data_size = width*height;
   if(flags&RVR_TEXTURE_MIPMAP)
      data_size+=data_size/3;
   switch(flags&RVR_TEXTURE_MULTI)
   {
   case RVR_TEXTURE_ANIM: data_size*=anim_frames; break;
   case RVR_TEXTURE_ROT4: data_size*=4; break;
   case RVR_TEXTURE_ROT8: data_size*=8; break;
   case RVR_TEXTURE_ROT16: data_size*=16; break;
   default: break;
   }

   rvr_textures[id] = RvR_malloc(sizeof(*rvr_textures[id]) + sizeof(*rvr_textures[id]->data) * data_size, "RvR texture");
   rvr_textures[id]->width = width;
   rvr_textures[id]->height = height;
   rvr_textures[id]->flags = flags;
   rvr_textures[id]->miplevels = 0;
   rvr_textures[id]->tex.exp = UINT32_MAX;
   rvr_textures[id]->anim_frames = anim_frames;
   rvr_textures[id]->anim_speed = anim_speed;
   for(size_t i = 0; i<data_size; i++)
      rvr_textures[id]->data[i] = RvR_rw_read_u8(&rw);
   RvR_mem_tag_set(rvr_textures[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(rvr_textures[id], (void **)&rvr_textures[id]);
   if(flags&RVR_TEXTURE_MIPMAP)
      rvr_textures[id]->miplevels = RvR_min(RvR_log2(width),RvR_log2(height));

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
   rvr_textures[id]->flags = 0;
   rvr_textures[id]->anim_frames = 0;
   rvr_textures[id]->anim_speed = 0;
   rvr_textures[id]->tex.exp = UINT32_MAX;
   rvr_textures[id]->data[0] = 0;

   RvR_mem_tag_set(rvr_textures[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(rvr_textures[id], (void **)&rvr_textures[id]);
}

void RvR_texture_create(uint16_t id, int width, int height)
{
   RvR_texture_free(id);

   if(rvr_textures==NULL)
   {
      rvr_textures = RvR_malloc(sizeof(*rvr_textures) * (UINT16_MAX + 1), "RvR texture cache");
      memset(rvr_textures, 0, sizeof(*rvr_textures) * (UINT16_MAX + 1));
   }

   rvr_textures[id] = RvR_malloc(sizeof(*rvr_textures[id]) + sizeof(*rvr_textures[id]->data) * width * height, "RvR texture");
   rvr_textures[id]->width = width;
   rvr_textures[id]->height = height;
   rvr_textures[id]->flags = 0;
   rvr_textures[id]->anim_frames = 0;
   rvr_textures[id]->anim_speed = 0;
}

void RvR_texture_free(uint16_t id)
{
   if(rvr_textures==NULL||rvr_textures[id]==NULL)
      return;

   RvR_free(rvr_textures[id]);
   rvr_textures[id] = NULL;
}
//-------------------------------------
