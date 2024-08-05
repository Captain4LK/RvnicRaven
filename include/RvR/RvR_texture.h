/*
RvnicRaven - texture managment

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_TEXTURE_H_

#define _RVR_TEXTURE_H_

//Texture flags
#define RVR_TEXTURE_MULTI     UINT32_C(0x7)
#define    RVR_TEXTURE_ANIM   UINT32_C(0x1)
#define    RVR_TEXTURE_ROT4   UINT32_C(0x2)
#define    RVR_TEXTURE_ROT8   UINT32_C(0x3)
#define    RVR_TEXTURE_ROT16  UINT32_C(0x4)
#define RVR_TEXTURE_MIPMAP    UINT32_C(0x8)

typedef struct
{
   int32_t width;
   int32_t height;

   uint32_t exp; //for mipmaps, non mipmaped dimension is dim*2^exp
   uint8_t *data; //width*height
}RvR_texture;

//Internal storage
typedef struct
{
   int32_t width;
   int32_t height;

   uint32_t flags;
   uint8_t anim_speed;
   uint8_t anim_frames;
   uint32_t miplevels;

   //Reserved data for returning from RvR_texture_get()
   RvR_texture tex;

   //size depends on having mipmaps and a multi flag being set
   uint8_t data[];
}RvR_itexture;

//All *texture_get* functions will NEVER return NULL
//When a texture doesn't exist, an empty 1x1 texture will be returned
//If you are out of memory, the program will crash instead
RvR_texture *RvR_texture_get(uint16_t id); //Returned data is marked cache, to set static, set the itexture as static. Texture is only valid until next get of same id
RvR_texture *RvR_texture_get_mipmap(uint16_t id, uint32_t level); //Same as RvR_texture_get(), but tries to find specified mipmap level
RvR_itexture *RvR_itexture_get(uint16_t id);  //Returned data is marked cache

void         RvR_texture_create(uint16_t id, int32_t width, int32_t height); //These textures need to be manually managed
void         RvR_texture_free(uint16_t id);

#endif
