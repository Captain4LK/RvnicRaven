/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

typedef struct
{
   int width;
   int height;
   uint8_t *data;
}Sprite_pal;

typedef struct
{
   uint16_t width;
   uint16_t height;
   uint32_t len;
   uint32_t row_offsets[768];
   uint8_t *data;
}Sprite_graphics;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void print_help(char **argv);
static Sprite_pal *texture_load(const char *path, const char *path_pal, uint8_t trans_index);
static Palette *palette_pal(FILE *f);
static Palette *palette_png(FILE *f);
static Palette *palette_gpl(FILE *f);
static Palette *palette_hex(FILE *f);
static Sprite_rgb *image_load(FILE *f);
static int chartoi(char in);

static Sprite_rgb *sprite_rgb_create(int width, int height);
static Sprite_pal *sprite_pal_create(int width, int height);
static void sprite_rgb_destroy(Sprite_rgb *s);
static void sprite_pal_destroy(Sprite_pal *s);

static void path_ext(const char *path, char ext[64]);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   struct optparse_long longopts[] =
   {
      {"in", 'i', OPTPARSE_REQUIRED},
      {"out", 'o', OPTPARSE_REQUIRED},
      {"pal", 'p', OPTPARSE_REQUIRED},
      {"type", 't', OPTPARSE_REQUIRED},
      {"help", 'h', OPTPARSE_NONE},
      {0},
   };

   const char *path_in = NULL;
   const char *path_out = NULL;
   const char *path_pal = NULL;
   const char *type = "sprite";

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
         path_pal = options.optarg;
         break;
      case 't':
         type = options.optarg;
         break;
      case '?':
         fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
         exit(EXIT_FAILURE);
         break;
      }
   }

   if(path_in==NULL)
   {
      RvR_log("input texture not specified, try %s -h for more info\n", argv[0]);
      return 0;
   }
   if(path_out==NULL)
   {
      RvR_log("output texture not specified, try %s -h for more info\n", argv[0]);
      return 0;
   }
   if(path_pal==NULL)
   {
      RvR_log("palette not specified, try %s -h for more info\n", argv[0]);
      return 0;
   }

   Sprite_pal *sp = texture_load(path_in, path_pal,0);

   if(strcmp(type,"wall")==0)
   {
      if(sp->width!=32||sp->height!=32*4)
      {
         RvR_log("error: wall dimensions must be 32x(32*4)\n");
         return EXIT_FAILURE;
      }
   }
   else if(strcmp(type,"floor")==0)
   {
      if(sp->width!=32||sp->height!=32)
      {
         RvR_log("error: floor dimensions must be 32x32\n");
         return EXIT_FAILURE;
      }
   }
   else if(strcmp(type,"slope")==0)
   {
      if(sp->width!=32||sp->height!=12*32)
      {
         RvR_log("error: slope dimensions must be 32x(12*32)\n");
         return EXIT_FAILURE;
      }
   }
   else if(strcmp(type,"sslope")==0)
   {
      if(sp->width!=32||sp->height!=24*32)
      {
         RvR_log("error: sslope dimensions must be 32x(24*32)\n");
         return EXIT_FAILURE;
      }
   }
   else if(strcmp(type,"block")==0)
   {
      if(sp->width!=32||sp->height!=32)
      {
         RvR_log("error: block dimensions must be 32x32\n");
         return EXIT_FAILURE;
      }
   }
   else if(strcmp(type,"sprite")==0)
   {
      if(sp->width>128||sp->height>128)
      {
         RvR_log("error: sprite dimensions must be less than 128x128\n");
         return EXIT_FAILURE;
      }
   }
   else
   {
      RvR_log("error: invalid type, must be one of [wall,floor,slope,sslope,block,sprite]\n");
      return EXIT_FAILURE;
   }

   //Build horizontal stripes
   Sprite_graphics grp = {0};
   grp.width = (uint16_t)sp->width;
   grp.height = (uint16_t)sp->height;
   uint32_t pos = 0;
   for(int y = 0;y<sp->height;y++)
   {
      grp.row_offsets[y] = pos;
      int count_pos = pos;
      RvR_array_push(grp.data,0);
      pos++;

      int start = 0;
      int run = 0;
      int count = 0;
      for(int x = 0;x<sp->width;x++)
      {
         //Skip transparent
         if(sp->data[y*sp->width+x]==0)
         {
            if(run>0)
            {
               RvR_array_push(grp.data,(uint8_t)start);
               RvR_array_push(grp.data,(uint8_t)run);
               pos+=2+run;
               for(int i = 0;i<run;i++)
                  RvR_array_push(grp.data,sp->data[y*sp->width+start+i]);
               count++;
            }
            run = 0;
            start = x+1;
            continue;
         }

         run++;
      }

      if(run>0)
      {
         RvR_array_push(grp.data,(uint8_t)start);
         RvR_array_push(grp.data,(uint8_t)run);
         pos+=2+run;
         for(int i = 0;i<run;i++)
            RvR_array_push(grp.data,sp->data[y*sp->width+start+i]);
         count++;
      }

      grp.data[count_pos] = count;
   }
   grp.len = (uint32_t)RvR_array_length(grp.data);

   sprite_pal_destroy(sp);

   //Compress and write to file
   RvR_rw cin;
   RvR_rw cout;
   size_t len = 4+grp.len+2+4+4*grp.height;
   int off_len = grp.height;
   uint8_t *mem = RvR_malloc(len, "compression buffer");
   RvR_rw_init_mem(&cin, mem, len, 0);
   RvR_rw_write_u16(&cin, 0);
   RvR_rw_write_u16(&cin, grp.width);
   RvR_rw_write_u16(&cin, grp.height);
   RvR_rw_write_u32(&cin, grp.len);
   for(int i = 0;i<off_len;i++)
      RvR_rw_write_u32(&cin, grp.row_offsets[i]);
   for(int i = 0;i<grp.len;i++)
      RvR_rw_write_u8(&cin,grp.data[i]);
   RvR_rw_init_path(&cout, path_out, "wb");
   RvR_crush_compress(&cin, &cout, 10);
   RvR_rw_close(&cin);
   RvR_rw_close(&cout);

   RvR_free(mem);
   RvR_array_free(grp.data);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
           "%s --in filename --out filename --pal filename [OPTIONS]\n"
           "   -i, --in          input texture path\n"
           "   -o, --out         output texture path\n"
           "   -p, --pal         palette to use for assigning indices (.pal,.png,.hex,.gpl\n"
           "   -t, --type        type of graphics, must be one of [wall,floor,sprite,slope,sslope,block]; sprite by default\n",
           argv[0], argv[0]);
}

static Sprite_pal *texture_load(const char *path, const char *path_pal, uint8_t trans_index)
{
   char ext[64] = {0};
   path_ext(path_pal, ext);
   Palette *pal = NULL;
   FILE *fpal = fopen(path_pal, "r");
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

   if(pal==NULL)
   {
      RvR_log_line("texture_load", "failed to load palette '%s'", path_pal);
      exit(-1);
   }

   FILE *ftex_in = fopen(path, "rb");
   Sprite_rgb *tex_in = image_load(ftex_in);
   if(ftex_in!=NULL)
      fclose(ftex_in);
   Sprite_pal *tex_out = sprite_pal_create(tex_in->width, tex_in->height);

   for(int i = 0; i<tex_in->width * tex_in->height; i++)
   {
      int min_dist = INT_MAX;
      uint8_t min_index = 0;
      Color color = tex_in->data[i];
      if(color.a<=128)
      {
         tex_out->data[i] = trans_index;
         continue;
      }

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
      tex_out->data[i] = min_index;
   }

   RvR_free(pal);
   sprite_rgb_destroy(tex_in);

   return tex_out;
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
         p->colors[c].r = (uint8_t)RvR_max(0,RvR_min(255,r));
         p->colors[c].g = (uint8_t)RvR_max(0,RvR_min(255,g));
         p->colors[c].b = (uint8_t)RvR_max(0,RvR_min(255,b));
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
      p->colors[c].r = (uint8_t)RvR_max(0,RvR_min(255,chartoi(buffer[0]) * 16 + chartoi(buffer[1])));
      p->colors[c].g = (uint8_t)RvR_max(0,RvR_min(255,chartoi(buffer[2]) * 16 + chartoi(buffer[3])));
      p->colors[c].b = (uint8_t)RvR_max(0,RvR_min(255,chartoi(buffer[4]) * 16 + chartoi(buffer[5])));
      p->colors[c].a = 255;
      c++;
   }
   p->colors_used = c;

   return p;
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

static Sprite_pal *sprite_pal_create(int width, int height)
{
   Sprite_pal *s = RvR_malloc(sizeof(*s), "Indexed sprite");
   s->width = width;
   s->height = height;
   s->data = RvR_malloc(sizeof(*s->data) * s->width * s->height, "Indexed sprite pixel data");

   return s;
}

static void sprite_rgb_destroy(Sprite_rgb *s)
{
   if(s==NULL)
      return;

   if(s->data!=NULL)
      RvR_free(s->data);
   RvR_free(s);
}

static void sprite_pal_destroy(Sprite_pal *s)
{
   if(s==NULL)
      return;

   if(s->data!=NULL)
      RvR_free(s->data);
   RvR_free(s);
}

static void path_ext(const char *path, char ext[64])
{
   if(path==NULL)
      return;

   ext[0] = '\0';

   char *last_dot = strrchr(path,'.');

   //No dot, or string is '.' or '..' --> no extension
   if(last_dot==NULL||!strcmp(path,".")||!strcmp(path,".."))
      return;

   //slash after dot --> no extension
   if(last_dot[1]=='/'||last_dot[1]=='\\')
      return;

   strncpy(ext,last_dot+1,63);
   ext[63] = '\0';
}
//-------------------------------------
