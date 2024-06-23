/*
RvnicRaven retro game engine

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "texture.h"
#include "map.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Texture_list texture_list = {0};
Texture_list_used texture_list_used = {0};
uint16_t texture_sky = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void texture_list_create()
{
   if(texture_list.data==NULL)
   {
      texture_list.data_size = 16;
      texture_list.data = RvR_malloc(sizeof(*texture_list.data) * texture_list.data_size, "ported texture list");
   }
   texture_list.data_used = 0;
   texture_sky = 0;

   for(unsigned i = 0; i<=UINT16_MAX; i++)
   {
      char tmp[16];
      snprintf(tmp, 16, "TEX%05d", i);
      if(!RvR_lump_exists(tmp))
         continue;

      texture_list.data[texture_list.data_used++] = i;
      if(texture_list.data_used==texture_list.data_size)
      {
         texture_list.data_size += 16;
         texture_list.data = RvR_realloc(texture_list.data, sizeof(*texture_list.data) * texture_list.data_size, "ported texture list grow");
      }
   }
}

void texture_list_used_create()
{
   for(unsigned i = 0; i<map->wall_count; i++)
   {
      texture_list_used_add(map->walls[i].tex_lower);
      texture_list_used_add(map->walls[i].tex_upper);
      texture_list_used_add(map->walls[i].tex_mid);
   }

   for(unsigned i = 0; i<map->sector_count; i++)
   {
      texture_list_used_add(map->sectors[i].floor_tex);
      texture_list_used_add(map->sectors[i].ceiling_tex);
   }
}

void texture_list_used_add(uint16_t tex)
{
   for(unsigned i = 0; i<TEXTURE_MRU_SIZE; i++)
   {
      //Place back to front
      if(texture_list_used.data[i]==tex)
      {
         if(i!=texture_list_used.data_last)
         {
            unsigned index = texture_list_used_wrap(i);
            while(index!=texture_list_used.data_last)
            {
               texture_list_used.data[texture_list_used_wrap(index)] = texture_list_used.data[texture_list_used_wrap(index + 1)];
               index = texture_list_used_wrap(index + 1);
            }
            texture_list_used.data[texture_list_used.data_last] = tex;
         }

         return;
      }
   }

   texture_list_used.data_last = texture_list_used_wrap(texture_list_used.data_last + 1);
   texture_list_used.data[texture_list_used.data_last] = tex;
}

int texture_valid(uint16_t tex)
{
   RvR_texture *texture = RvR_texture_get(tex);
   if(texture==NULL)
      return 0;

   if(texture->width==1 << RvR_log2(texture->width)&&texture->height==1 << RvR_log2(texture->height))
      return 1;

   return 0;
}
//-------------------------------------
