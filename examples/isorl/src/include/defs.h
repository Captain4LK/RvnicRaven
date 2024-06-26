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
   uint16_t sprite;
   char type[16];
   char name[32];

   uint64_t tags;
}ItemDef;

typedef struct
{
   char type[16];

   char adjective[32];
   uint32_t density;
   uint8_t remap0;
   uint8_t remap1;
   uint8_t remap2;
   uint8_t remap3;

   uint64_t tags;
}MaterialDef;

typedef struct
{
   char name[32];
   int16_t sprite_index;
   uint64_t tags;
   int16_t child;
   int16_t next;
}BodypartDef;

typedef struct
{
   char type[16];
   uint16_t sprite_sheet;
   uint16_t bodypart_count;
   BodypartDef *bodyparts;
}BodyDef;

typedef struct
{
   char type[16];
   const BodyDef *male_body;
   const BodyDef *female_body;
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
#define DEF_BODY_VITAL           (UINT64_C(1) << 0)
#define DEF_BODY_SLOT_UPPER      (UINT64_C(1) << 1)
#define DEF_BODY_SLOT_LOWER      (UINT64_C(1) << 2)
#define DEF_BODY_SLOT_HEAD       (UINT64_C(1) << 3)
#define DEF_BODY_SLOT_HAND       (UINT64_C(1) << 4)
#define DEF_BODY_SLOT_FOOT       (UINT64_C(1) << 5)
#define DEF_BODY_GRASP           (UINT64_C(1) << 6)
//-------------------------------------

//Material
//-------------------------------------
#define DEF_MAT_FROM_CREATURE   (UINT64_C(1) << 0)
//-------------------------------------

//Item
//-------------------------------------
#define DEF_ITEM_EQUIP_UPPER     UINT64_C(0x1)
#define DEF_ITEM_EQUIP_LOWER     UINT64_C(0x2)
#define DEF_ITEM_EQUIP_HEAD      UINT64_C(0x4)
#define DEF_ITEM_EQUIP_HAND      UINT64_C(0x8)
#define DEF_ITEM_EQUIP_FOOT      UINT64_C(0x10)
#define DEF_ITEM_LAYER_UNDER     UINT64_C(0x20)
#define DEF_ITEM_LAYER_OVER      UINT64_C(0x40)
#define DEF_ITEM_LAYER_ARMOR     UINT64_C(0x80)
#define DEF_ITEM_LAYER_BACK      UINT64_C(0x100)
#define DEF_ITEM_SLOT_CONTAINER  UINT64_C(0x200)
//-------------------------------------

#endif
