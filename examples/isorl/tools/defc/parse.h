/*
RvnicRaven - iso roguelike: definition compiler

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _PARSE_H_

#define _PARSE_H_

#include "ini.h"

typedef struct
{
   RvR_rw rw;
   ini_stream ini;
   int error;
}Parser;

typedef enum
{
   MKR_INVALID = 0,                    //
   MKR_MATERIAL_START = 1,             //type:char[16]
   MKR_MATERIAL_END = 2,               //
   MKR_ADJECTIVE = 3,                  //adjective:char[32]
   MKR_DENSITY = 4,                    //density in mg/cm3: u32
   MKR_ITEM_START = 5,                 //type:char[16]
   MKR_ITEM_END = 6,                   //
   MKR_NAME = 7,                       //name:char[32]
   MKR_BODY_START = 8,                 //type:char[16]
   MKR_BODY_END = 9,                   //
   MKR_ENTITY_START = 10,              //type:char[16]
   MKR_ENTITY_END = 11,                //
   MKR_GROUP_START = 12,               //type:char[16]
   MKR_GROUP_END = 13,                 //
   MKR_BODYPART_START = 14,            //
   MKR_BODYPART_END = 15,              //
   MKR_BODY = 16,                      //bodytype:char[16]
   MKR_VITAL = 17,                     //
   MKR_SLOT_HEAD = 18,                 //
   MKR_SLOT_UPPER = 19,                //
   MKR_SLOT_LOWER = 20,                //
   MKR_SLOT_HAND = 21,                 //
   MKR_SLOT_FOOT = 22,                 //
   MKR_GRASP = 23,                     //
   MKR_SPRITE_INDEX = 24,              //sprite_index:i16
   MKR_SPRITE_SHEET = 25,              //sprite_sheet:u16
   MKR_MALE_START = 26,                //
   MKR_MALE_END = 27,                  //
   MKR_FEMALE_START = 28,              //
   MKR_FEMALE_END = 29,                //
   MKR_FROM_CREATURE = 30,             //
   MKR_SPRITE = 31,                    //sprite:u16
   MKR_REMAP_0 = 32,                   //remap0:u8
   MKR_REMAP_1 = 33,                   //remap1:u8
   MKR_REMAP_2 = 34,                   //remap2:u8
   MKR_REMAP_3 = 35,                   //remap3:u8
   MKR_SLOT_CONTAINER = 36,            //
   MKR_EQUIP_HEAD = 37,                //
   MKR_EQUIP_UPPER = 38,               //
   MKR_EQUIP_LOWER = 39,               //
   MKR_EQUIP_HAND = 40,                //
   MKR_EQUIP_FOOT = 41,                //
   MKR_LAYER_UNDER = 42,               //
   MKR_LAYER_OVER = 43,                //
   MKR_LAYER_ARMOR = 44,               //
   MKR_LAYER_BACK = 45,                //
}Marker;

void parser_init(Parser *p, const char *path_out);
void parser_close(Parser *p);
int parse_file(Parser *p, const char *path);

#endif
