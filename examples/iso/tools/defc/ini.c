/*
RvnicRaven - iso roguelike: definition compiler

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <RvR/RvR.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "ini.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void ini_stream_open(ini_stream *ini, const char *path)
{
   RvR_rw_init_path(&ini->stream,path,"r");
   ini->path = RvR_malloc(strlen(path)+1,"ini_stream path");
   strcpy(ini->path,path);
   ini->line = 1;
   ini->key = NULL;
   ini->value = NULL;
}

void ini_stream_close(ini_stream *ini)
{
   RvR_array_free(ini->key);
   RvR_array_free(ini->value);
   ini->key = NULL;
   ini->value = NULL;
   RvR_free(ini->path);
}

ini_type ini_stream_next(ini_stream *ini)
{
   char next = RvR_rw_read_u8(&ini->stream);
   for(;next&&!RvR_rw_eof(&ini->stream);next = RvR_rw_read_u8(&ini->stream))
   {
      //Skip whitespace
      if(next&&(next==' '||next=='\t'||next=='\n'))
      {
         if(next=='\n')
            ini->line++;
      }
      //Comment: skip
      else if(next==';')
      {
         while(next&&next!='\n')
            next = RvR_rw_read_u8(&ini->stream);
         ini->line++;
      }
      //Section
      else if(next=='[')
      {
         RvR_array_length_set(ini->value,0);

         next = RvR_rw_read_u8(&ini->stream);
         if(next==']')
            return INI_SECTION_END;
         while(next!=']')
         {
            //Check for escaped bracket
            if(next=='\\')
            {
               next = RvR_rw_read_u8(&ini->stream);
               if(next!=']')
                  RvR_array_push(ini->value,'\\');
            }

            //Newline --> error
            if(next=='\n')
            {
               RvR_log("%s:%d: unterminated section, expected ']', but got newline\n",ini->path,ini->line);
               return INI_ERROR;
            }

            RvR_array_push(ini->value,next);

            next = RvR_rw_read_u8(&ini->stream);
         }

         RvR_array_push(ini->value,'\0');

         return INI_SECTION;
      }
      //key:value pair or tag (if no '=' in line)
      else if(next>' '&&next<='z')
      {
         RvR_array_length_set(ini->key,0);
         RvR_array_length_set(ini->value,0);

         //Store in key until '='
         while(next&&next!='=')
         {
            RvR_array_push(ini->key,next);
            next = RvR_rw_read_u8(&ini->stream);
            
            if(next=='\n')
            {
               RvR_array_push(ini->key,'\0');
               ini->line++;
               return INI_TAG;
            }
         }

         //Store in value until newline
         next = RvR_rw_read_u8(&ini->stream);
         while(next&&next!='\n'&&next!='\r')
         {
            RvR_array_push(ini->value,next);
            next = RvR_rw_read_u8(&ini->stream);
         }
         ini->line++;

         RvR_array_push(ini->key,'\0');
         RvR_array_push(ini->value,'\0');

         return INI_KEY;
      }
      //Invalid: skip
      else
      {
         RvR_log("%s:%d invalid character '%c'(%d)\n",ini->path,ini->line,next,next);
         while(next&&next!='\n')
            next = RvR_rw_read_u8(&ini->stream);
         ini->line++;
      }
   }

   return INI_END;
}

const char *ini_stream_value(const ini_stream *ini)
{
   return ini->value;
}

const char *ini_stream_key(const ini_stream *ini)
{
   return ini->key;
}
//-------------------------------------
