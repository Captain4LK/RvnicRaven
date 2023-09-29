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
   MKR_MATERIAL_START = 0,             //name:char[16]
   MKR_MATERIAL_END = 1,               //
   MKR_ADJECTIVE = 2,                  //adjective:char[32]
   MKR_DENSITY = 3,                    //density in mg/cm3: u32
}Marker;

void parser_init(Parser *p, const char *path_out);
void parser_close(Parser *p);
int parse_file(Parser *p, const char *path);

#endif
