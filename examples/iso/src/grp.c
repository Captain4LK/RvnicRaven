/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "grp.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static GRP_floor **floor_grps;
static GRP_block **block_grps;
static GRP_wall  **wall_grps;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

GRP_floor *grp_floor_get(uint16_t id)
{
   uint8_t *mem_pak = NULL;
   uint8_t *mem_decomp = NULL;

   RvR_error_check(id<2048, "grp_floor_get", "floor grp id must be less than 2048\n");

   if(floor_grps==NULL)
   {
      floor_grps = RvR_malloc(sizeof(*floor_grps) * 2048, "iso floor grp array");
      memset(floor_grps, 0, sizeof(*floor_grps) * 2048);
   }

   if(floor_grps[id]==NULL)
   {
      char tmp[64];
      sprintf(tmp, "FLOR%04d", id);

      size_t size_in;
      size_t size_out;
      mem_pak = RvR_lump_get(tmp, &size_in);

      RvR_error_check(size_in!=0, "grp_floor_get", "FLOR%04d not found\n", id);

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
      RvR_error_check(version==0, "grp_floor_get", "'FLOR%04d' has invalid version %d, expected version 0\n", id, version);

      uint16_t width = RvR_rw_read_u16(&rw);
      RvR_error_check(width==32, "grp_floor_get", "'FLOR%04d' has invalid width, expected '32', got '%d'\n", id, width);
      uint16_t height = RvR_rw_read_u16(&rw);
      RvR_error_check(height==32, "grp_floor_get", "'FLOR%04d' has invalid height, expected '32', got '%d'\n", id, height);
      uint32_t len = RvR_rw_read_u32(&rw);

      floor_grps[id] = RvR_malloc(sizeof(*floor_grps[id]) + sizeof(*floor_grps[id]->data) * len, "floor grp");
      for(int i = 0; i<32; i++)
         floor_grps[id]->row_offsets[i] = RvR_rw_read_u32(&rw);
      for(int i = 0; i<len; i++)
         floor_grps[id]->data[i] = RvR_rw_read_u8(&rw);

      RvR_rw_close(&rw);

      RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);
   }

   RvR_mem_tag_set(floor_grps[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(floor_grps[id], (void **)&floor_grps[id]);
   return floor_grps[id];

RvR_err:
   if(mem_decomp!=NULL)
      RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);
   if(mem_pak!=NULL)
      RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   return NULL;
}

GRP_block *grp_block_get(uint16_t id)
{
   uint8_t *mem_pak = NULL;
   uint8_t *mem_decomp = NULL;

   RvR_error_check(id<2048, "grp_block_get", "block grp id must be less than 2048\n");

   if(block_grps==NULL)
   {
      block_grps = RvR_malloc(sizeof(*block_grps) * 2048, "iso block grp array");
      memset(block_grps, 0, sizeof(*block_grps) * 2048);
   }

   if(block_grps[id]==NULL)
   {
      char tmp[64];
      sprintf(tmp, "BLCK%04d", id);

      size_t size_in;
      size_t size_out;
      mem_pak = RvR_lump_get(tmp, &size_in);

      RvR_error_check(size_in!=0, "grp_block_get", "BLCK%04d not found\n", id);

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
      RvR_error_check(version==0, "grp_block_get", "'BLCK%04d' has invalid version %d, expected version 0\n", id, version);

      uint16_t width = RvR_rw_read_u16(&rw);
      RvR_error_check(width==32, "grp_block_get", "'BLCK%04d' has invalid width, expected '32', got '%d'\n", id, width);
      uint16_t height = RvR_rw_read_u16(&rw);
      RvR_error_check(height==32, "grp_block_get", "'BLCK%04d' has invalid height, expected '32', got '%d'\n", id, height);
      uint32_t len = RvR_rw_read_u32(&rw);

      block_grps[id] = RvR_malloc(sizeof(*block_grps[id]) + sizeof(*block_grps[id]->data) * len, "block grp");
      for(int i = 0; i<32; i++)
         block_grps[id]->row_offsets[i] = RvR_rw_read_u32(&rw);
      for(int i = 0; i<len; i++)
         block_grps[id]->data[i] = RvR_rw_read_u8(&rw);

      RvR_rw_close(&rw);

      RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);
   }

   RvR_mem_tag_set(block_grps[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(block_grps[id], (void **)&block_grps[id]);
   return block_grps[id];

RvR_err:
   if(mem_decomp!=NULL)
      RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);
   if(mem_pak!=NULL)
      RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   return NULL;
}

GRP_wall  *grp_wall_get(uint16_t id)
{
}
//-------------------------------------
