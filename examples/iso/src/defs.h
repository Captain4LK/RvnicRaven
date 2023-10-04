/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _DEFS_H_

#define _DEFS_H_

#include <stdint.h>

typedef struct
{
   char type[16];

   char name[32];
}ItemDef;

typedef struct
{
   char type[16];

   char adjective[32];
   uint32_t density;
}MaterialDef;

typedef struct
{
   char name[32];
   uint64_t tags;
   int16_t child;
   int16_t next;
}BodypartDef;

typedef struct
{
   char type[16];
   uint16_t bodypart_count;
   BodypartDef *bodyparts;
}BodyDef;

typedef struct
{
   char type[16];
   const BodyDef *body;
}EntityDef;

typedef struct
{
   char type[16];
}GroupDef;

//DO NOT LOAD ANY DEFS WHILE A GAME IS ACTIVE
//I.E: ONLY LOAD THEM AT STARTUP
void defs_init(void);
void defs_load(const char *path);

const MaterialDef *defs_get_material(const char *type);
const ItemDef *defs_get_item(const char *type);
const BodyDef *defs_get_body(const char *type);
const EntityDef *defs_get_entity(const char *type);
const GroupDef *defs_get_group(const char *type);

//Tags

//Body
//-------------------------------------
#define DEF_BODY_VITAL     UINT64_C(1<<0)
//-------------------------------------

#endif
