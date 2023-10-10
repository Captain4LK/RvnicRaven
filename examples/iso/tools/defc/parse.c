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

#define field_u16(parser,kind,mkr,key,value) \
   do \
   { \
      RvR_rw_write_u32(&(parser)->rw,mkr); \
      RvR_rw_write_u16(&(parser)->rw,(uint16_t)strtol((value),NULL,10)); \
   } \
   while(0)
      //RvR_log("%s:%d: error: material name '%s' too long, max 15 characters\n",p->ini.path,p->ini.line,name);
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void parse_material(Parser *p, const char *name);
static void parse_item(Parser *p, const char *name);
static void parse_body(Parser *p, const char *name);
static void parse_bodypart(Parser *p);
static void parse_entity(Parser *p, const char *name);
static void parse_group(Parser *p, const char *name);
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
            parse_material(p,name);
         else if(strcmp("item",type)==0)
            parse_item(p,name);
         else if(strcmp("body",type)==0)
            parse_body(p,name);
         else if(strcmp("entity",type)==0)
            parse_entity(p,name);
         else if(strcmp("group",type)==0)
            parse_group(p,name);
         else
         {
            RvR_log("%s:%d: error: unknow type '%s'\n",p->ini.path,p->ini.line,type);
            goto err;
         }
         break;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   if(next==INI_ERROR)
      goto err;

   if(type!=NULL) RvR_array_free(type);
   if(name!=NULL) RvR_array_free(name);

   ini_stream_close(&p->ini);
   return 0;

err:
   if(type!=NULL) RvR_array_free(type);
   if(name!=NULL) RvR_array_free(name);

   ini_stream_close(&p->ini);
   return 1;
}

static void parse_material(Parser *p, const char *name)
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
   while(next!=INI_END&&next!=INI_ERROR&&next!=INI_SECTION_END)
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
      //case INI_SECTION:
         //RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);
         //return INI_SECTION;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);

   return;

err:
   return;
}

static void parse_item(Parser *p, const char *name)
{
   RvR_rw_write_u32(&p->rw,MKR_ITEM_START);
   if(strlen(name)>15)
   {
      RvR_log("%s:%d: error: item name '%s' too long, max 15 characters\n",p->ini.path,p->ini.line,name);
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
   while(next!=INI_END&&next!=INI_ERROR&&next!=INI_SECTION_END)
   {
      switch(next)
      {
      case INI_KEY:
      {
         const char *key = ini_stream_key(&p->ini);
         const char *value = ini_stream_value(&p->ini);
         if(strcmp(key,"name")==0)
            field_string(p,"item",MKR_NAME,32,key,value);
         else
         {
            RvR_log("%s:%d: warning: unknown material attribute '%s'\n",p->ini.path,p->ini.line-1,key);
         }
      }
      break;
      case INI_TAG:
         break;
      //Next section
      //case INI_SECTION:
         //RvR_rw_write_u32(&p->rw,MKR_ITEM_END);
         //return INI_SECTION;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   RvR_rw_write_u32(&p->rw,MKR_ITEM_END);

   return;

err:
   return;
}

static void parse_body(Parser *p, const char *name)
{
   RvR_rw_write_u32(&p->rw,MKR_BODY_START);
   if(strlen(name)>15)
   {
      RvR_log("%s:%d: error: body name '%s' too long, max 15 characters\n",p->ini.path,p->ini.line,name);
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
   while(next!=INI_END&&next!=INI_ERROR&&next!=INI_SECTION_END)
   {
      switch(next)
      {
      case INI_KEY:
      {
         const char *key = ini_stream_key(&p->ini);
         const char *value = ini_stream_value(&p->ini);

         if(strcmp(key,"sprite_sheet")==0)
            field_u16(p,"body",MKR_SPRITE_SHEET,key,value);
         //else if(strcmp(key,"density")==0)
            //field_u32(p,"material",MKR_DENSITY,key,value);
         else
         {
            RvR_log("%s:%d: warning: unknown body attribute '%s'\n",p->ini.path,p->ini.line-1,key);
         }
      }
      break;
      case INI_TAG:
         break;
      case INI_SECTION:
         const char *value = ini_stream_value(&p->ini);
         if(strcmp(value,"bodypart")==0)
         {
            parse_bodypart(p);
         }
         else
         {
            RvR_log("%s:%d: error : unknown body subsection '%s'\n",p->ini.path,p->ini.line-1,value);
            goto err;
         }
         //RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);
         //return INI_SECTION;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   RvR_rw_write_u32(&p->rw,MKR_BODY_END);

   return;

err:
   return;
}

static void parse_bodypart(Parser *p)
{
   RvR_rw_write_u32(&p->rw,MKR_BODYPART_START);

   ini_type next = ini_stream_next(&p->ini);
   while(next!=INI_END&&next!=INI_ERROR&&next!=INI_SECTION_END)
   {
      switch(next)
      {
      case INI_KEY:
      {
         const char *key = ini_stream_key(&p->ini);
         const char *value = ini_stream_value(&p->ini);
         if(strcmp(key,"name")==0)
            field_string(p,"bodypart",MKR_NAME,32,key,value);
         else if(strcmp(key,"sprite_index")==0)
            field_u16(p,"bodypart",MKR_SPRITE_INDEX,key,value);
         //if(strcmp(key,"adjective")==0)
            //field_string(p,"material",MKR_ADJECTIVE,32,key,value);
         //else if(strcmp(key,"density")==0)
            //field_u32(p,"material",MKR_DENSITY,key,value);
         else
         {
            RvR_log("%s:%d: warning: unknown bodypart attribute '%s'\n",p->ini.path,p->ini.line-1,key);
         }
      }
      break;
      case INI_TAG:
      {
         const char *key = ini_stream_key(&p->ini);
         if(strcmp(key,"vital")==0)
            RvR_rw_write_u32(&p->rw,MKR_VITAL); 
         else if(strcmp(key,"slot_upper")==0)
            RvR_rw_write_u32(&p->rw,MKR_SLOT_UPPER); 
         else if(strcmp(key,"slot_lower")==0)
            RvR_rw_write_u32(&p->rw,MKR_SLOT_LOWER); 
         else if(strcmp(key,"slot_head")==0)
            RvR_rw_write_u32(&p->rw,MKR_SLOT_HEAD); 
         else if(strcmp(key,"slot_hand")==0)
            RvR_rw_write_u32(&p->rw,MKR_SLOT_HAND); 
         else if(strcmp(key,"slot_foot")==0)
            RvR_rw_write_u32(&p->rw,MKR_SLOT_FOOT); 
         else if(strcmp(key,"grasp")==0)
            RvR_rw_write_u32(&p->rw,MKR_GRASP); 
         else
            RvR_log("%s:%d: warning: unknown bodypart tag '%s'\n",p->ini.path,p->ini.line-1,key);
      }
         break;
      case INI_SECTION:
         const char *value = ini_stream_value(&p->ini);
         if(strcmp(value,"bodypart")==0)
         {
            parse_bodypart(p);
         }
         else
         {
            RvR_log("%s:%d: error : unknown material subsection '%s'\n",p->ini.path,p->ini.line-1,value);
            goto err;
         }
         //RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);
         //return INI_SECTION;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   RvR_rw_write_u32(&p->rw,MKR_BODYPART_END);

   return;

err:
   return;
}

static void parse_entity(Parser *p, const char *name)
{
   RvR_rw_write_u32(&p->rw,MKR_ENTITY_START);
   if(strlen(name)>15)
   {
      RvR_log("%s:%d: error: entity name '%s' too long, max 15 characters\n",p->ini.path,p->ini.line,name);
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
   int current_sex = -1;
   while(next!=INI_END&&next!=INI_ERROR&&(current_sex!=-1||next!=INI_SECTION_END))
   {
      switch(next)
      {
      case INI_KEY:
      {
         const char *key = ini_stream_key(&p->ini);
         const char *value = ini_stream_value(&p->ini);
         //if(strcmp(key,"adjective")==0)
            //field_string(p,"material",MKR_ADJECTIVE,32,key,value);
         //else if(strcmp(key,"density")==0)
            //field_u32(p,"material",MKR_DENSITY,key,value);
         if(strcmp(key,"body")==0)
            field_string(p,"entity",MKR_BODY,16,key,value);
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
      {
         const char *value = ini_stream_value(&p->ini);
         if(strcmp(value,"male")==0)
         {
            RvR_rw_write_u32(&p->rw,MKR_MALE_START);
            current_sex = 0;
         }
         else if(strcmp(value,"female")==0)
         {
            RvR_rw_write_u32(&p->rw,MKR_FEMALE_START);
            current_sex = 1;
         }
         else
         {
            RvR_log("%s:%d: warning: unknown entity subsection'%s'\n",p->ini.path,p->ini.line-1,value);
         }
      }
         break;
      case INI_SECTION_END:
         if(current_sex==0)
            RvR_rw_write_u32(&p->rw,MKR_MALE_END);
         else if(current_sex==1)
            RvR_rw_write_u32(&p->rw,MKR_FEMALE_END);
         current_sex = -1;
         break;
         //RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);
         //return INI_SECTION;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   RvR_rw_write_u32(&p->rw,MKR_ENTITY_END);

   return;

err:
   return;
}

static void parse_group(Parser *p, const char *name)
{
   RvR_rw_write_u32(&p->rw,MKR_GROUP_START);
   if(strlen(name)>15)
   {
      RvR_log("%s:%d: error: group name '%s' too long, max 15 characters\n",p->ini.path,p->ini.line,name);
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
   while(next!=INI_END&&next!=INI_ERROR&&next!=INI_SECTION_END)
   {
      switch(next)
      {
      case INI_KEY:
      {
         const char *key = ini_stream_key(&p->ini);
         const char *value = ini_stream_value(&p->ini);
         //if(strcmp(key,"adjective")==0)
            //field_string(p,"material",MKR_ADJECTIVE,32,key,value);
         //else if(strcmp(key,"density")==0)
            //field_u32(p,"material",MKR_DENSITY,key,value);
         //else
         {
            RvR_log("%s:%d: warning: unknown material attribute '%s'\n",p->ini.path,p->ini.line-1,key);
         }
      }
      break;
      case INI_TAG:
         break;
      //Next section
      //case INI_SECTION:
         //RvR_rw_write_u32(&p->rw,MKR_MATERIAL_END);
         //return INI_SECTION;
      default:
         break;
      }

      next = ini_stream_next(&p->ini);
   }

   RvR_rw_write_u32(&p->rw,MKR_GROUP_END);

   return;

err:
   return;
}
//-------------------------------------
