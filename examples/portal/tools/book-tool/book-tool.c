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
#include <inttypes.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CUTE_PATH_IMPLEMENTATION
#include "cute_path.h"

#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   uint8_t r, g, b, a;
}Color;

typedef struct
{
   int colors_used;
   Color colors[256];
}Palette;

typedef struct
{
   int width;
   int height;
   Color *data;
}Sprite_rgb;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void print_help(char **argv);

static char *config_ini_parse(const char *path);
static char *config_ini(const char *s);
static Palette *palette_pal(FILE *f);
static Palette *palette_png(FILE *f);
static Palette *palette_gpl(FILE *f);
static Palette *palette_hex(FILE *f);
static Sprite_rgb *image_load(FILE *f);
static int chartoi(char in);

static Sprite_rgb *sprite_rgb_create(int width, int height);
static void sprite_rgb_destroy(Sprite_rgb *s);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   struct optparse_long longopts[] =
   {
      {"in", 'i', OPTPARSE_REQUIRED},
      {"out", 'o', OPTPARSE_REQUIRED},
      {"palette", 'p', OPTPARSE_REQUIRED},
      {"help", 'h', OPTPARSE_NONE},
      {0},
   };

   const char *path_in = NULL;
   const char *path_out = NULL;
   const char *path_palette = NULL;

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
      case 'p':
         path_palette = options.optarg;
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

   if(path_palette==NULL)
   {
      RvR_log("palette not specified, try %s -h\n",argv[0]);
      return EXIT_FAILURE;
   }

   char ext[33] = {0};
   path_pop_ext(path_palette, NULL, ext);
   Palette *pal = NULL;
   FILE *fpal = fopen(path_palette, "r");
   if(fpal!=NULL)
   {
      if(strncmp(ext, "pal", 32)==0)
         pal = palette_pal(fpal);
      else if(strncmp(ext, "png", 32)==0)
         pal = palette_png(fpal);
      else if(strncmp(ext, "gpl", 32)==0)
         pal = palette_gpl(fpal);
      else if(strncmp(ext, "hex", 32)==0)
         pal = palette_hex(fpal);
      fclose(fpal);
   }

   char path_tex[512];
   path_pop_ext(path_in, path_tex, NULL);
   strcat(path_tex,".png");
   FILE *f = fopen(path_tex,"rb");
   Sprite_rgb *tex_book = image_load(f);
   fclose(f);
   if(tex_book->width!=4||tex_book->height!=16)
   {
      RvR_log("invalid book texture dimension, must be 4x16\n");
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
      else if(strcmp(ident,"author")==0)
         snprintf(book_author,64,"%s",iter);
      else if(strcmp(ident,"date")==0)
         snprintf(book_date,16,"%s",iter);
      else if(strcmp(ident,"words")==0)
         book_words = (uint32_t)strtol(iter,NULL,10);
      else if(strcmp(ident,"case")==0)
         book_case = (uint8_t)strtol(iter,NULL,10);
      else if(strcmp(ident,"shelf")==0)
         book_shelf = (uint8_t)strtol(iter,NULL,10);
      else if(strcmp(ident,"slot")==0)
         book_slot = (uint8_t)strtol(iter,NULL,10);

      while(*iter++);
   }

   RvR_rw rw = {0};
   RvR_rw_init_path(&rw,path_out,"wb");
   RvR_rw_write(&rw,book_title,1,64);
   RvR_rw_write(&rw,book_author,1,64);
   RvR_rw_write(&rw,book_date,1,16);
   RvR_rw_write_u32(&rw,book_words);
   RvR_rw_write_u8(&rw,book_case);
   RvR_rw_write_u8(&rw,book_shelf);
   RvR_rw_write_u8(&rw,book_slot);
   for(int i = 0;i<4*16;i++)
   {
      int min_dist = INT_MAX;
      uint8_t min_index = 0;
      Color color = tex_book->data[i];
      //if(color.a<=128)
      //{
         //tex_out->data[i] = trans_index;
         //continue;
      //}

      for(int j = 0; j<pal->colors_used; j++)
      {
         Color pal_color = pal->colors[j];
         int dist_r = pal_color.r - color.r;
         int dist_g = pal_color.g - color.g;
         int dist_b = pal_color.b - color.b;
         int dist = dist_r * dist_r + dist_g * dist_g + dist_b * dist_b;
         if(dist<min_dist)
         {
            min_dist = dist;
            min_index = (uint8_t)j;
         }
      }

      RvR_rw_write_u8(&rw,min_index);
   }
   RvR_rw_close(&rw);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
           "%s --in filename --out filename\n"
           "   -i, --in          input book definition path\n"
           "   -o, --out         output book path\n"
           "   -p, --palette     palette for book sprite\n",
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

static Palette *palette_pal(FILE *f)
{
   Palette *p = RvR_malloc(sizeof(*p), "Palette");

   fscanf(f, "JASC-PAL\n0100\n%d\n", &p->colors_used);
   char buffer[512];
   for(int i = 0; fgets(buffer, 512, f); i++)
      sscanf(buffer, "%" SCNu8 "%" SCNu8 "%" SCNu8 "\n", &p->colors[i].r, &p->colors[i].g, &p->colors[i].b);
   return p;
}

static Palette *palette_png(FILE *f)
{
   Sprite_rgb *s = image_load(f);
   Palette *p = RvR_malloc(sizeof(*p), "Palette");
   memset(p, 0, sizeof(*p));
   p->colors_used = RvR_min(256, s->width * s->height);
   for(int i = 0; i<p->colors_used; i++)
      p->colors[i] = s->data[i];
   sprite_rgb_destroy(s);

   return p;
}

static Palette *palette_gpl(FILE *f)
{
   Palette *p = RvR_malloc(sizeof(*p), "Palette");
   if(!p)
      return NULL;
   memset(p, 0, sizeof(*p));
   char buffer[512];
   int c = 0;
   int r, g, b;

   while(fgets(buffer, 512, f))
   {
      if(buffer[0]=='#')
         continue;
      if(sscanf(buffer, "%d %d %d", &r, &g, &b)==3)
      {
         p->colors[c].r = (uint8_t)RvR_max(0, RvR_min(255, r));
         p->colors[c].g = (uint8_t)RvR_max(0, RvR_min(255, g));
         p->colors[c].b = (uint8_t)RvR_max(0, RvR_min(255, b));
         p->colors[c].a = 255;
         c++;
      }
   }
   p->colors_used = c;

   return p;
}

static Palette *palette_hex(FILE *f)
{

   Palette *p = RvR_malloc(sizeof(*p), "Palette");
   if(!p)
      return NULL;
   memset(p, 0, sizeof(*p));
   char buffer[512];
   int c = 0;

   while(fgets(buffer, 512, f))
   {
      p->colors[c].r = (uint8_t)RvR_max(0, RvR_min(255, chartoi(buffer[0]) * 16 + chartoi(buffer[1])));
      p->colors[c].g = (uint8_t)RvR_max(0, RvR_min(255, chartoi(buffer[2]) * 16 + chartoi(buffer[3])));
      p->colors[c].b = (uint8_t)RvR_max(0, RvR_min(255, chartoi(buffer[4]) * 16 + chartoi(buffer[5])));
      p->colors[c].a = 255;
      c++;
   }
   p->colors_used = c;

   return p;
}

static void sprite_rgb_destroy(Sprite_rgb *s)
{
   if(s==NULL)
      return;

   if(s->data!=NULL)
      RvR_free(s->data);
   RvR_free(s);
}

static Sprite_rgb *image_load(FILE *f)
{
   unsigned char *data = NULL;
   int width = 1;
   int height = 1;
   Sprite_rgb *out;

   data = stbi_load_from_file(f, &width, &height, NULL, 4);
   if(data==NULL)
   {
      RvR_log("Failed to load image\n");
      return sprite_rgb_create(1, 1);
   }

   out = sprite_rgb_create(width, height);
   memcpy(out->data, data, width * height * sizeof(*out->data));

   stbi_image_free(data);

   return out;
}

//Helper function for palette_hex
static int chartoi(char in)
{
   if(in>='0'&&in<='9')
      return in - '0';
   if(in>='a'&&in<='f')
      return in - 'a' + 10;
   if(in>='A'&&in<='F')
      return in - 'A' + 10;
   return 0;
}

static Sprite_rgb *sprite_rgb_create(int width, int height)
{
   Sprite_rgb *s = RvR_malloc(sizeof(*s), "RGB sprite");
   s->width = width;
   s->height = height;
   s->data = RvR_malloc(sizeof(*s->data) * s->width * s->height, "RGB sprite pixel data");

   return s;
}
//-------------------------------------
