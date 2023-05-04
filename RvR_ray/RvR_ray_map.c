/*
RvnicRaven - raycast map

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR_ray_config.h"
#include "RvR/RvR_ray.h"
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

RvR_ray_map *RvR_ray_map_create(uint16_t width, uint16_t height)
{
   if(width==0)
      width = 1;
   if(height==0)
      height = 1;

   RvR_ray_map *map = RvR_malloc(sizeof(*map),"RvR_ray map");
   memset(map,0,sizeof(*map));

   map->width = width;
   map->height = height;
   map->sky_tex = 0;
   map->sprite_count = 0;
   map->floor = RvR_malloc(map->width*map->height*sizeof(*map->floor),"RvR_ray map floor");
   map->ceiling = RvR_malloc(map->width*map->height*sizeof(*map->ceiling),"RvR_ray map ceiling" );
   map->floor_tex = RvR_malloc(map->width*map->height*sizeof(*map->floor_tex),"RvR_ray map floor_tex");
   map->ceil_tex = RvR_malloc(map->width*map->height*sizeof(*map->ceil_tex),"RvR_ray map ceil_tex");
   map->wall_ftex = RvR_malloc(map->width*map->height*sizeof(*map->wall_ftex),"RvR_ray map wall_ftex");
   map->wall_ctex = RvR_malloc(map->width*map->height*sizeof(*map->wall_ctex),"RvR_ray map wall_ctex");

   memset(map->floor,0,map->width*map->height*sizeof(*map->floor));
   memset(map->floor_tex,0,map->width*map->height*sizeof(*map->floor_tex));
   memset(map->ceil_tex,0,map->width*map->height*sizeof(*map->ceil_tex));
   memset(map->wall_ftex,0,map->width*map->height*sizeof(*map->wall_ftex));
   memset(map->wall_ctex,0,map->width*map->height*sizeof(*map->wall_ctex));

   for(int y = 0;y<map->height;y++)
      for(int x = 0;x<map->width;x++)
         map->ceiling[y*map->width+x] = 32*8192;

   return map;
}

void RvR_ray_map_free(RvR_ray_map *map)
{
   if(map->floor!=NULL)
      RvR_free(map->floor);
   if(map->ceiling!=NULL)
      RvR_free(map->ceiling);
   if(map->floor_tex!=NULL)
      RvR_free(map->floor_tex);
   if(map->ceil_tex!=NULL)
      RvR_free(map->ceil_tex);
   if(map->wall_ftex!=NULL)
      RvR_free(map->wall_ftex);
   if(map->wall_ctex!=NULL)
      RvR_free(map->wall_ctex);
   if(map->sprites!=NULL)
      RvR_free(map->sprites);

   map->floor = NULL;
   map->floor_tex = NULL;
   map->ceil_tex= NULL;
   map->ceiling = NULL;
   map->wall_ftex = NULL;
   map->wall_ctex = NULL;
   map->sprites = NULL;
   map->width = 0;
   map->height = 0;
   map->sprite_count = 0;
}

RvR_ray_map *RvR_ray_map_load(uint16_t id)
{
   char tmp[64];
   sprintf(tmp,"MAP%05d",id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   mem_pak = RvR_lump_get(tmp,&size_in);
   RvR_mem_tag_set(mem_pak,RVR_MALLOC_STATIC);
   RvR_rw rw_decomp;
   RvR_rw_init_const_mem(&rw_decomp,mem_pak,size_in);
   mem_decomp = RvR_crush_decompress(&rw_decomp,&size_out);
   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);
   RvR_mem_tag_set(mem_pak,RVR_MALLOC_CACHE);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw,mem_decomp,size_out);
   RvR_ray_map *map = RvR_ray_map_load_rw(&rw);
   RvR_rw_close(&rw);

   RvR_mem_tag_set(mem_decomp,RVR_MALLOC_CACHE);

   return map;
}

RvR_ray_map *RvR_ray_map_load_path(const char *path)
{
   RvR_error_check(path!=NULL,"RvR_ray_map_load_path","argument 'path' must be non-NULL\n");

   int32_t size = 0;
   RvR_rw rw_decomp;
   RvR_rw_init_path(&rw_decomp,path,"rb");
   uint8_t *mem = RvR_crush_decompress(&rw_decomp,&size);
   RvR_mem_tag_set(mem,RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw,mem,size);
   RvR_ray_map *map = RvR_ray_map_load_rw(&rw);
   RvR_rw_close(&rw);

   RvR_mem_tag_set(mem,RVR_MALLOC_CACHE);

   return map;

RvR_err:
   return NULL;;
}

RvR_ray_map *RvR_ray_map_load_rw(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL,"RvR_ray_map_load_rw","argument 'rw' must be non-NULL\n");

   //Read and check version
   uint16_t version = RvR_rw_read_u16(rw);
   RvR_error_check(version==1,"RvR_ray_map_load_rw","Invalid version '%d', expected version '1'\n",version);

   //Read sky texture
   uint16_t sky_tex = RvR_rw_read_u16(rw);

   //Read level width and height
   uint16_t width = RvR_rw_read_u16(rw);
   uint16_t height = RvR_rw_read_u16(rw);
   
   //Create map
   RvR_ray_map *map = RvR_ray_map_create(width,height);
   map->sky_tex = sky_tex;

   //Read sprite count
   map->sprite_count = RvR_rw_read_u32(rw);
   map->sprites = RvR_malloc(sizeof(*map->sprites)*map->sprite_count,"RvR_ray map sprites");

   //Read texture, floor and ceiling
   int32_t tile_count = map->width*map->height;
   for(int32_t i = 0;i<tile_count;i++) map->floor[i] = RvR_rw_read_u32(rw);
   for(int32_t i = 0;i<tile_count;i++) map->ceiling[i] = RvR_rw_read_u32(rw);
   for(int32_t i = 0;i<tile_count;i++) map->floor_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) map->ceil_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) map->wall_ftex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) map->wall_ctex[i] = RvR_rw_read_u16(rw);

   //Read sprites
   for(unsigned i = 0;i<map->sprite_count;i++)
   {
      map->sprites[i].x = RvR_rw_read_u32(rw);
      map->sprites[i].y = RvR_rw_read_u32(rw);
      map->sprites[i].z = RvR_rw_read_u32(rw);
      map->sprites[i].direction = RvR_rw_read_u32(rw);
      map->sprites[i].texture = RvR_rw_read_u16(rw);
      map->sprites[i].flags = RvR_rw_read_u32(rw);
      map->sprites[i].extra0 = RvR_rw_read_u32(rw);
      map->sprites[i].extra1 = RvR_rw_read_u32(rw);
      map->sprites[i].extra2 = RvR_rw_read_u32(rw);
   }

   return map;

RvR_err:
   return NULL;
}

void RvR_ray_map_save(const RvR_ray_map *map, const char *path)
{
   RvR_error_check(path!=NULL,"RvR_ray_map_save","argument 'path' must be non-NULL\n");

   //Calculate needed memory
   int size = 0;
   size+=2; //version
   size+=2; //map->sky_tex
   size+=2; //map->width
   size+=2; //map->height
   size+=4; //map->sprite_count
   size+=map->width*map->height*4; //map->floor
   size+=map->width*map->height*4; //map->ceiling
   size+=map->width*map->height*2; //map->floor_tex
   size+=map->width*map->height*2; //map->ceil_tex
   size+=map->width*map->height*2; //map->wall_ftex
   size+=map->width*map->height*2; //map->wall_ctex
   size+=map->sprite_count*sizeof(*map->sprites);

   uint8_t *mem = RvR_malloc(size,"RvR_ray map save buffer");
   RvR_rw rw = {0};
   RvR_rw_init_mem(&rw,mem,size,size);

   //version
   RvR_rw_write_u16(&rw,1);

   //sky texture
   RvR_rw_write_u16(&rw,map->sky_tex);

   //width and height
   RvR_rw_write_u16(&rw,map->width);
   RvR_rw_write_u16(&rw,map->height);

   //sprite count
   RvR_rw_write_u32(&rw,map->sprite_count);

   //floor height
   for(int i = 0;i<map->width*map->height;i++)
      RvR_rw_write_u32(&rw,map->floor[i]);

   //ceiling height
   for(int i = 0;i<map->width*map->height;i++)
      RvR_rw_write_u32(&rw,map->ceiling[i]);

   //floor texture
   for(int i = 0;i<map->width*map->height;i++)
      RvR_rw_write_u16(&rw,map->floor_tex[i]);

   //ceiling texture
   for(int i = 0;i<map->width*map->height;i++)
      RvR_rw_write_u16(&rw,map->ceil_tex[i]);

   //wall ftex
   for(int i = 0;i<map->width*map->height;i++)
      RvR_rw_write_u16(&rw,map->wall_ftex[i]);

   //wall ctex
   for(int i = 0;i<map->width*map->height;i++)
      RvR_rw_write_u16(&rw,map->wall_ctex[i]);

   //sprites
   for(unsigned i = 0;i<map->sprite_count;i++)
   {
      RvR_rw_write_u32(&rw,map->sprites[i].x);
      RvR_rw_write_u32(&rw,map->sprites[i].y);
      RvR_rw_write_u32(&rw,map->sprites[i].z);
      RvR_rw_write_u32(&rw,map->sprites[i].direction);
      RvR_rw_write_u16(&rw,map->sprites[i].texture);
      RvR_rw_write_u32(&rw,map->sprites[i].flags);
      RvR_rw_write_u32(&rw,map->sprites[i].extra0);
      RvR_rw_write_u32(&rw,map->sprites[i].extra1);
      RvR_rw_write_u32(&rw,map->sprites[i].extra2);
   }

   //Compress and write to disk
   RvR_rw in;
   RvR_rw out;
   RvR_rw_init_mem(&in,mem,size,size);
   RvR_rw_init_path(&out,path,"wb");
   RvR_crush_compress(&in,&out,0);
   RvR_rw_close(&in);
   RvR_rw_close(&out);

   //Free temp buffer
   RvR_rw_close(&rw);
   RvR_free(mem);

RvR_err:
   return;
}

int RvR_ray_map_inbounds(const RvR_ray_map *map, int16_t x, int16_t y)
{
   if(x>=0&&x<map->width&&y>=0&&y<map->height)
      return 1;
   return 0;
}

RvR_fix16 RvR_ray_map_floor_height_at(const RvR_ray_map *map, int16_t x, int16_t y)
{
   if(RvR_ray_map_inbounds(map,x,y))
      return map->floor[y*map->width+x];

   return 127*8192;
}

RvR_fix16 RvR_ray_map_ceiling_height_at(const RvR_ray_map *map, int16_t x, int16_t y)
{
   if(RvR_ray_map_inbounds(map,x,y))
      return map->ceiling[y*map->width+x];

   return 127*8192;
}

uint16_t RvR_ray_map_floor_tex_at(const RvR_ray_map *map, int16_t x, int16_t y)
{
   if(RvR_ray_map_inbounds(map,x,y))
      return map->floor_tex[y*map->width+x];

   return map->sky_tex;
}

uint16_t RvR_ray_map_ceil_tex_at(const RvR_ray_map *map, int16_t x, int16_t y)
{
   if(RvR_ray_map_inbounds(map,x,y))
      return map->ceil_tex[y*map->width+x];

   return map->sky_tex;
}

uint16_t RvR_ray_map_wall_ftex_at(const RvR_ray_map *map, int16_t x, int16_t y)
{
   if(RvR_ray_map_inbounds(map,x,y))
      return map->wall_ftex[y*map->width+x];

   return map->sky_tex;
}

uint16_t RvR_ray_map_wall_ctex_at(const RvR_ray_map *map, int16_t x, int16_t y)
{
   if(RvR_ray_map_inbounds(map,x,y))
      return map->wall_ctex[y*map->width+x];

   return map->sky_tex;
}

//TODO: remove?
RvR_fix16 RvR_ray_map_floor_height_at_us(const RvR_ray_map *map, int16_t x, int16_t y)
{
   return map->floor[y*map->width+x];
}

RvR_fix16 RvR_ray_map_ceiling_height_at_us(const RvR_ray_map *map, int16_t x, int16_t y)
{
   return map->ceiling[y*map->width+x];
}

uint16_t RvR_ray_map_floor_tex_at_us(const RvR_ray_map *map, int16_t x, int16_t y)
{
   return map->floor_tex[y*map->width+x];
}

uint16_t RvR_ray_map_ceil_tex_at_us(const RvR_ray_map *map, int16_t x, int16_t y)
{
   return map->ceil_tex[y*map->width+x];
}

uint16_t RvR_ray_map_wall_ftex_at_us(const RvR_ray_map *map, int16_t x, int16_t y)
{
   return map->wall_ftex[y*map->width+x];
}

uint16_t RvR_ray_map_wall_ctex_at_us(const RvR_ray_map *map, int16_t x, int16_t y)
{
   return map->wall_ctex[y*map->width+x];
}

void RvR_ray_map_floor_height_set(RvR_ray_map *map, int16_t x, int16_t y, RvR_fix16 height)
{
   if(RvR_ray_map_inbounds(map,x,y))
      map->floor[y*map->width+x] = height;
}

void RvR_ray_map_ceiling_height_set(RvR_ray_map *map, int16_t x, int16_t y, RvR_fix16 height)
{
   if(RvR_ray_map_inbounds(map,x,y))
      map->ceiling[y*map->width+x] = height;
}

void RvR_ray_map_floor_tex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex)
{
   if(RvR_ray_map_inbounds(map,x,y))
      map->floor_tex[y*map->width+x] = tex;
}

void RvR_ray_map_ceil_tex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex)
{
   if(RvR_ray_map_inbounds(map,x,y))
      map->ceil_tex[y*map->width+x] = tex;
}

void RvR_ray_map_wall_ftex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex)
{
   if(RvR_ray_map_inbounds(map,x,y))
      map->wall_ftex[y*map->width+x] = tex;
}

void RvR_ray_map_wall_ctex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex)
{
   if(RvR_ray_map_inbounds(map,x,y))
      map->wall_ctex[y*map->width+x] = tex;
}
//-------------------------------------
