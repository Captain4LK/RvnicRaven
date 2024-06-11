/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void print_help(char **argv);

static char *config_ini_parse(const char *path);
static char *config_ini(const char *s);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   struct optparse_long longopts[] =
   {
      {"in", 'i', OPTPARSE_REQUIRED},
      {"out", 'o', OPTPARSE_REQUIRED},
      {"help", 'h', OPTPARSE_NONE},
      {0},
   };

   const char *path_in = NULL;
   const char *path_out = NULL;

   int option;
   struct optparse options;
   optparse_init(&options, argv);
   while((option = optparse_long(&options, longopts, NULL))!=-1)
   {
      switch(option)
      {
      case 'h':
         print_help(argv);
         exit(EXIT_SUCCESS);
      case 'i':
         path_in = options.optarg;
         break;
      case 'o':
         path_out = options.optarg;
         break;
      case '?':
         fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
         exit(EXIT_FAILURE);
         break;
      }
   }

   if(path_in==NULL)
   {
      RvR_log("input definition not specified, try %s -h\n",argv[0]);
      return EXIT_FAILURE;
   }

   if(path_out==NULL)
   {
      RvR_log("output definition not specified, try %s -h\n",argv[0]);
      return EXIT_FAILURE;
   }

   char *ini = config_ini_parse(path_in);
   char book_title[64] = "???";
   char book_author[64] = "???";
   char book_date[16] = "???";
   uint32_t book_words = 1024;
   uint8_t book_case = 0;
   uint8_t book_shelf = 0;
   uint8_t book_slot = 0;

   char *iter = NULL;
   for(iter = ini; iter[0];)
   {
      char *ident = iter;
      while(*iter++);

      if(strcmp(ident,"title")==0)
         snprintf(book_title,64,"%s",iter);
      else if(strcmp(iter,"author")==0)
         snprintf(book_author,64,"%s",iter);
      else if(strcmp(iter,"date")==0)
         snprintf(book_date,16,"%s",iter);
      else if(strcmp(iter,"words")==0)
         book_words = (uint32_t)strtol(iter,NULL,10);
      else if(strcmp(iter,"case")==0)
         book_case = (uint8_t)strtol(iter,NULL,10);
      else if(strcmp(iter,"shelf")==0)
         book_shelf = (uint8_t)strtol(iter,NULL,10);
      else if(strcmp(iter,"slot")==0)
         book_slot = (uint8_t)strtol(iter,NULL,10);

      while(*iter++);
   }

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
           "%s --in filename --out filename\n"
           "   -i, --in          input book definition path\n"
           "   -o, --out         output book path\n",
           argv[0], argv[0]);
}

static char *config_ini_parse(const char *path)
{
   RvR_rw rw = {0};
   RvR_rw_init_path(&rw, path, "r");

   if(!RvR_rw_valid(&rw))
   {
      return NULL;
   }

   RvR_rw_seek(&rw, 0, SEEK_END);
   long size = RvR_rw_tell(&rw);
   RvR_rw_seek(&rw, 0, SEEK_SET);
   char *buffer_in = RvR_malloc(sizeof(*buffer_in) * (size + 1), "config_read file string");
   RvR_rw_read(&rw, buffer_in, sizeof(*buffer_in) * size, 1);
   buffer_in[size] = '\0';
   RvR_rw_close(&rw);

   char *kv = config_ini(buffer_in);
   RvR_free(buffer_in);

   return kv;
}

//Ini parser by r-lyeh: https://github.com/r-lyeh/tinybits
//Original header:
// ini+, extended ini format
// - rlyeh, public domain
//
// # spec
//
//   ; line comment
//   [details]          ; map section name (optional)
//   user=john          ; key and value (mapped here as details.user=john)
//   +surname=doe jr.   ; sub-key and value (mapped here as details.user.surname=doe jr.)
//   color=240          ; key and value |
//   color=253          ; key and value |> array: color[0], color[1] and color[2]
//   color=255          ; key and value |
//   color=             ; remove key/value(s)
//   color=white        ; recreate key; color[1] and color[2] no longer exist
//   []                 ; unmap section
//   -note=keys may start with symbols (except plus and semicolon)
//   -note=linefeeds are either \r, \n or \r\n.
//   -note=utf8 everywhere.
//
static char *config_ini(const char *s)
{
   char *map = NULL;
   int mapcap = 0;
   int maplen = 0;
   enum
   {
      DEL, REM, TAG, KEY, SUB, VAL
   } fsm = DEL;
   const char *cut[6] = {0};
   const char *end[6] = {0};

   while(*s)
   {
      while(*s&&(*s==' '||*s =='\t'||*s=='\r'||*s=='\n')) ++s;

      if(*s ==';') cut[fsm = REM] = ++s;
      else if(*s=='[') cut[fsm = TAG] = ++s;
      else if(*s=='+') cut[fsm = SUB] = ++s;
      else if(*s=='=') cut[fsm = VAL] = ++s;
      else if(*s>' '&&*s<='z'&&*s!=']') cut[fsm = KEY] = cut[SUB] = end[SUB] = s;
      else { ++s; continue; }

      if(fsm==REM) { while(*s&&*s!='\r'&&*s!='\n') ++s; }
      else if(fsm==TAG) { while(*s&&*s!='\r'&&*s!='\n'&&*s!=']') ++s; end[fsm] = s; }
      else if(fsm==KEY) { while(*s&&*s> ' '&&*s<='z'&&*s!='=') ++s; end[fsm] = s; }
      else if(fsm==SUB) { while(*s&&*s> ' '&&*s<='z'&&*s!='=') ++s; end[fsm] = s; }
      else if(fsm==VAL)
      {
         char buf[256] = {0};
         char *key = NULL;
         int reqlen = 0;

         while(*s && *s >= ' ' && *s <= 'z' && *s != ';') ++s;
         end[fsm] = s;
         while(end[fsm][-1]==' ') --end[fsm];
         key = buf;
         if(end[TAG] - cut[TAG])key += sprintf(key, "%.*s.", (int)(end[TAG] - cut[TAG]), cut[TAG]);
         if(end[KEY] - cut[KEY])key += sprintf(key, "%.*s", (int)(end[KEY] - cut[KEY]), cut[KEY]);
         if(end[SUB] - cut[SUB])key += sprintf(key, ".%.*s", (int)(end[SUB] - cut[SUB]), cut[SUB]);
         reqlen = (key - buf) + 1 + (end[VAL] - cut[VAL]) + 1 + 1;
         if((reqlen + maplen)>=mapcap) { map = RvR_realloc(map, (mapcap += reqlen + 512), "config_ini map"); }
         sprintf(map + maplen, "%.*s%c%.*s%c%c", (int)(key - buf), buf, 0, (int)(end[VAL] - cut[VAL]), cut[VAL], 0, 0);
         maplen += reqlen - 1;
      }
   }

   return map;
}
//-------------------------------------
