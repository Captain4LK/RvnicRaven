/*
RvnicRaven - iso roguelike: definition compiler

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <RvR/RvR.h>
#include <stdlib.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "ini.h"
#include "parse.h"
//-------------------------------------

//#defines
#define field_string(parser,kind,mkr,max_len,key,value) \
   do \
   { \
   if(strlen(value)>(max_len)-1)   \
   { \
      RvR_log("%s:%d: error: %s %s '%s' too long, max %d characters\n",(parser)->ini.path,(parser)->ini.line,(kind),(key),(value),(max_len)); \
      goto err; \
   } \
   RvR_rw_write_u32(&(parser)->rw,mkr); \
   const char *value_str = (value); \
   for(int i = 0;i<(max_len);i++) \
   { \
      RvR_rw_write_u8(&(parser)->rw,*value_str); \
      if(*value_str) value_str++; \
   } \
   } \
   while(0)

#define field_u32(parser,kind,mkr,key,value) \
   do \
   { \
      RvR_rw_write_u32(&(parser)->rw,mkr); \
      RvR_rw_write_u32(&(parser)->rw,(uint32_t)strtol((value),NULL,10)); \
   } \
   while(0)
      //RvR_log("%s:%d: error: material name '%s' too long, max 15 characters\n",p->ini.path,p->ini.line,name);
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static ini_type parse_material(Parser *p, const char *name);
//-------------------------------------

//Function implementations

void parser_init(Parser *p, const char *path_out)
{
   p->error = 0;
   RvR_rw_init_path(&p->rw,path_out,"wb");
}

void parser_close(Parser *p)
{
   RvR_rw_close(&p->rw);
}

int parse_file(Parser *p, const char *path)
{
   if(p->error)
      return 1;

   ini_stream_open(&p->ini,path);
   char *type = NULL;
   char *name = NULL;

   ini_type next = ini_stream_next(&p->ini);
   while(next!=INI_END&&next!=INI_ERROR)
   {
      switch(next)
      {
      case INI_TAG:
      case INI_KEY:
         RvR_log("%s:%d: warning: %s outside of section won't have an effect\n",p->ini.path,p->ini.line-1,next==INI_TAG?"tag":"key");
         next = ini_stream_next(&p->ini);
         break;
      case INI_SECTION:
         //Split into type:name
         RvR_array_length_set(type,0);
         RvR_array_length_set(name,0);
         const char *str = ini_stream_value(&p->ini);
         for(;*str&&*str!=':';str++)
            RvR_array_push(type,*str);
         if(*str=='\0'||*(str+1)=='\0')
         {
            RvR_log("%s:%d: error: invalid section format, expected type:name\n",p->ini.path,p->ini.line);
            goto err;
         }
         str++;
         for(;*str;str++)
            RvR_array_push(name,*str);

         RvR_array_push(type,'\0');
         RvR_array_push(name,'\0');

         if(strcmp("material",type)==0)
            next = parse_material(p,name);
         else
         {
            RvR_log("%s:%d: error: unknow type '%s'\n",p->ini.path,p->ini.line,type);
            goto err;
         }
         break;
      default:
         next = ini_stream_next(&p->ini);
         break;
      }
   }

   if(next==INI_ERROR)
      goto err;

   ini_stream_close(&p->ini);
   return 0;

err:
   ini_stream_close(&p->ini);
   return 1;
}

static ini_type parse_material(Parser *p, const char *name)
{
   RvR_rw_write_u32(&p->rw,MKR_MATERIAL_START);
   if(strlen(name)>15)
   {
      RvR_log("%s:%d: error: material name '%s' too long, max 15 characters\n",p->ini.path,p->ini.line,name);
      goto err;
   }

   const char *nstr = name;
   for(int i = 0;i<16;i++)
   {
      RvR_rw_write_u8(&p->rw,*nstr);
      if(*nstr)
         nstr++;
   }

   ini_type next = ini_stream_next(&p->ini);
   while(next!=INI_END&&next!=INI_ERROR)
   {
      switch(next)
      {
      case INI_KEY:
      {
         const char *key = ini_stream_key(&p->ini);
         const char *value = ini_stream_value(&p->ini);
         if(strcmp(key,"adjective")==0)
            field_string(p,"material",MKR_ADJECTIVE,32,key,value);
         else if(strcmp(key,"density")==0)
            field_u32(p,"material",MKR_DENSITY,key,value);
         else
         {
            RvR_log("%s:%d: warning: unknown material attribute '%s'\n",p->ini.path,p->ini.line-1,key);
         }
      }
      break;
      case INI_TAG:
         break;
      //Next section
      case INI_SECTION:
         RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);
         return INI_SECTION;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);

   return next;

err:
   return INI_ERROR;
}
//-------------------------------------
