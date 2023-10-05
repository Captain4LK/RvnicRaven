/*
RvnicRaven - iso roguelike: definition compiler

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
}Marker;

void parser_init(Parser *p, const char *path_out);
void parser_close(Parser *p);
int parse_file(Parser *p, const char *path);

#endif
