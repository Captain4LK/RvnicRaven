/*
RvnicRaven retro game engine

Written in 2021,2022,2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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

#define CUTE_PATH_IMPLEMENTATION
#include "cute_path.h"

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
typedef enum
{
   SPRITE_NONE = 0,
   SPRITE_WALL = 1,
   SPRITE_SPRITE = 2,
}Sprite_flag;

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
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   struct optparse_long longopts[] =
   {
      {"in", 'i', OPTPARSE_REQUIRED},
      {"out", 'o', OPTPARSE_REQUIRED},
      {"pal", 'p', OPTPARSE_REQUIRED},
      {"anim", 'a', OPTPARSE_REQUIRED},
      {"wall", 'w', OPTPARSE_NONE},
      {"sprite", 's', OPTPARSE_NONE},
      {"trans", 't', OPTPARSE_REQUIRED},
      {"help", 'h', OPTPARSE_NONE},
      {0},
   };

   const char *path_in = NULL;
   const char *path_out = NULL;
   const char *path_pal = NULL;
   uint64_t flags = SPRITE_NONE;
   uint8_t anim_range = 0;
   uint8_t anim_speed = 0;
   uint8_t trans_index = 0;
   uint32_t sflags = 0;

   int option;
   struct optparse options;
   optparse_init(&options, argv);
   while((option = optparse_long(&options, longopts, NULL))!=-1)
   {
      switch(option)
      {
      case 'w':
         flags |= SPRITE_WALL;
         break;
      case 's':
         flags |= SPRITE_SPRITE;
         break;
      case 'a':
         sscanf(options.optarg, "%" SCNu8 ",%" SCNu8, &anim_range, &anim_speed);
         sflags|=RVR_TEXTURE_ANIM;
         break;
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
         trans_index = (uint8_t)strtol(options.optarg, NULL, 10);
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

   Sprite_pal *sp = texture_load(path_in, path_pal, trans_index);

   int transposed = 0;
   if(flags & SPRITE_WALL)
   {
      Sprite_pal *csp = sprite_pal_create(sp->width, sp->height);
      memcpy(csp->data, sp->data, sizeof(*sp->data) * sp->width * sp->height);

      transposed = 1;
      for(int x = 0; x<sp->width; x++)
         for(int y = 0; y<sp->height; y++)
            sp->data[x * sp->height + y] = csp->data[y * csp->width + x];

      sprite_pal_destroy(csp);
   }
   else if(flags & SPRITE_SPRITE)
   {
      transposed = 1;
      Sprite_pal *csp = sprite_pal_create(sp->width, sp->height);
      memcpy(csp->data, sp->data, sizeof(*sp->data) * sp->width * sp->height);

      for(int x = 0; x<sp->width; x++)
         for(int y = 0; y<sp->height; y++)
            sp->data[x * sp->height + y] = csp->data[y * csp->width + x];

      sprite_pal_destroy(csp);
   }

   uint32_t data_size = sp->width*sp->height;
   if((((unsigned)sp->width)&((unsigned)sp->width-1))==0&&
      (((unsigned)sp->height)&((unsigned)sp->height-1))==0)
   {
      sflags|=RVR_TEXTURE_MIPMAP;
      data_size+=data_size/3;
   }

   char ext[33] = {0};
   path_pop_ext(path_pal, NULL, ext);
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

   //Compress and write to file
   RvR_rw cin;
   RvR_rw cout;
   size_t len = sizeof(uint8_t) * data_size + sizeof(int16_t) + 2 * sizeof(int8_t) + 3 * sizeof(int32_t);
   uint8_t *mem = RvR_malloc(len, "compression buffer");
   RvR_rw_init_mem(&cin, mem, len, len);
   RvR_rw_write_u16(&cin, 1);
   RvR_rw_write_u32(&cin, sp->width);
   RvR_rw_write_u32(&cin, sp->height);
   RvR_rw_write_u32(&cin, sflags);
   RvR_rw_write_u8(&cin, anim_speed);
   RvR_rw_write_u8(&cin, anim_range);
   for(int i = 0; i<sp->width * sp->height; i++)
      RvR_rw_write_u8(&cin, sp->data[i]);
   if(sflags&RVR_TEXTURE_MIPMAP)
   {
      unsigned mwidth = sp->width/2;
      unsigned mheight = sp->height/2;
      while(mwidth>=1&&mheight>=1)
      {
         if(transposed)
         {
            for(int x = 0;x<mwidth;x++)
            {
               for(int y = 0;y<mheight;y++)
               {
                  uint8_t c0,c1,c2,c3;
                  if(transposed)
                  {
                     c0 = sp->data[(x*2)*mheight*2+(y*2)];
                     c1 = sp->data[(x*2+1)*mheight*2+(y*2)];
                     c2 = sp->data[(x*2)*mheight*2+(y*2+1)];
                     c3 = sp->data[(x*2+1)*mheight*2+(y*2+1)];
                  }
                  else
                  {
                     c0 = sp->data[(y*2)*mwidth*2+(x*2)];
                     c1 = sp->data[(y*2+1)*mwidth*2+(x*2)];
                     c2 = sp->data[(y*2)*mwidth*2+(x*2+1)];
                     c3 = sp->data[(y*2+1)*mwidth*2+(x*2+1)];
                  }
                  uint32_t r = (pal->colors[c0].r+pal->colors[c1].r+pal->colors[c2].r+pal->colors[c3].r)/4;
                  uint32_t g = (pal->colors[c0].g+pal->colors[c1].g+pal->colors[c2].g+pal->colors[c3].g)/4;
                  uint32_t b = (pal->colors[c0].b+pal->colors[c1].b+pal->colors[c2].b+pal->colors[c3].b)/4;

                  int min_dist = INT_MAX;
                  uint8_t min_index = 0;
                  for(int j = 0; j<pal->colors_used; j++)
                  {
                     Color pal_color = pal->colors[j];
                     int dist_r = pal_color.r - r;
                     int dist_g = pal_color.g - g;
                     int dist_b = pal_color.b - b;
                     int dist = dist_r * dist_r + dist_g * dist_g + dist_b * dist_b;
                     if(dist<min_dist)
                     {
                        min_dist = dist;
                        min_index = (uint8_t)j;
                     }
                  }
                  if(transposed)
                     sp->data[x*mheight+y] = min_index;
                  else
                     sp->data[y*mwidth+x] = min_index;
               }
            }
         }
         else
         {
            for(int y = 0;y<mheight;y++)
            {
               for(int x = 0;x<mwidth;x++)
               {
                  uint8_t c0,c1,c2,c3;
                  if(transposed)
                  {
                     c0 = sp->data[(x*2)*mheight*2+(y*2)];
                     c1 = sp->data[(x*2+1)*mheight*2+(y*2)];
                     c2 = sp->data[(x*2)*mheight*2+(y*2+1)];
                     c3 = sp->data[(x*2+1)*mheight*2+(y*2+1)];
                  }
                  else
                  {
                     c0 = sp->data[(y*2)*mwidth*2+(x*2)];
                     c1 = sp->data[(y*2+1)*mwidth*2+(x*2)];
                     c2 = sp->data[(y*2)*mwidth*2+(x*2+1)];
                     c3 = sp->data[(y*2+1)*mwidth*2+(x*2+1)];
                  }
                  uint32_t r = (pal->colors[c0].r+pal->colors[c1].r+pal->colors[c2].r+pal->colors[c3].r)/4;
                  uint32_t g = (pal->colors[c0].g+pal->colors[c1].g+pal->colors[c2].g+pal->colors[c3].g)/4;
                  uint32_t b = (pal->colors[c0].b+pal->colors[c1].b+pal->colors[c2].b+pal->colors[c3].b)/4;

                  int min_dist = INT_MAX;
                  uint8_t min_index = 0;
                  for(int j = 0; j<pal->colors_used; j++)
                  {
                     Color pal_color = pal->colors[j];
                     int dist_r = pal_color.r - r;
                     int dist_g = pal_color.g - g;
                     int dist_b = pal_color.b - b;
                     int dist = dist_r * dist_r + dist_g * dist_g + dist_b * dist_b;
                     if(dist<min_dist)
                     {
                        min_dist = dist;
                        min_index = (uint8_t)j;
                     }
                  }
                  if(transposed)
                     sp->data[x*mheight+y] = min_index;
                  else
                     sp->data[y*mwidth+x] = min_index;
               }
            }
         }

         for(int i = 0;i<mwidth*mheight;i++)
            RvR_rw_write_u8(&cin,sp->data[i]);

         mwidth/=2;
         mheight/=2;
      }
   }
   RvR_rw_init_path(&cout, path_out, "wb");
   RvR_crush_compress(&cin, &cout, 10);
   RvR_rw_close(&cin);
   RvR_rw_close(&cout);

   //TODO(Captain4LK): generate mipmaps

   sprite_pal_destroy(sp);
   RvR_free(mem);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
           "%s --in filename --out filename --pal filename [OPTIONS]\n"
           "   -i, --in          input texture path\n"
           "   -o, --out         output texture path\n"
           "   -p, --pal         palette to use for assigning indices (.pal,.png,.hex,.gpl\n"
           "   -w, --wall        flag sprite as wall texture\n"
           "   -t, --trans       transparent palette index\n"
           "   -s, --sprite      flag sprite as sprite texture\n",
           argv[0], argv[0]);
}

static Sprite_pal *texture_load(const char *path, const char *path_pal, uint8_t trans_index)
{
   char ext[33] = {0};
   path_pop_ext(path, NULL, ext);

   path_pop_ext(path_pal, NULL, ext);
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
//-------------------------------------
