/*
RvnicRaven - texture managment

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_TEXTURE_H_

#define _RVR_TEXTURE_H_

typedef struct
{
   int32_t width;
   int32_t height;
   uint8_t data[];
}RvR_texture;

RvR_texture *RvR_texture_get(uint16_t id); //Pointer returned is only valid until next RvR_texture_get() call (unless the texture has been manually created)
void         RvR_texture_create(uint16_t id, int width, int height); //These textures need to be manually managed
void         RvR_texture_create_free(uint16_t id);

#endif
