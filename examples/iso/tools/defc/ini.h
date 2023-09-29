/*
RvnicRaven - iso roguelike: definition compiler

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _INI_H_

#define _INI_H_

#include <RvR/RvR.h>

typedef struct
{
   RvR_rw stream;
   char *path;
   int line;
   char *key;
   char *value;
}ini_stream;

typedef enum
{
   INI_SECTION,
   INI_KEY,
   INI_TAG,
   INI_END,
   INI_ERROR,
}ini_type;

void ini_stream_open(ini_stream *ini, const char *path);
void ini_stream_close(ini_stream *ini);
ini_type ini_stream_next(ini_stream *ini);
const char *ini_stream_value(const ini_stream *ini); //Only valid until next ini_stream_next() call
const char *ini_stream_key(const ini_stream *ini); //Only valid until next ini_stream_next() call

#endif
