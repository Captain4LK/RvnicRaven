/*
RvnicRaven - portal map 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR/RvR_portal.h"
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

RvR_port_map *RvR_port_map_create(void)
{
   RvR_port_map *map = RvR_malloc(sizeof(*map),"RvR_port map");
   memset(map,0,sizeof(*map));
   return map;
}

void RvR_port_map_save(const RvR_port_map *map, const char *path)
{
   //Calculate needed memory
   int size = 0;
   size += 2; //version
   size += 4; //map->wall_count
   size += 4; //map->sector_count
   size+=map->wall_count*(4+4+4+2+2+2+2+2+2); //Walls
   size+=map->sector_count*(2+2+4+4+2+2+4); //Sectors

   uint8_t *mem = RvR_malloc(size, "RvR_port map save buffer");
   RvR_rw rw = {0};
   RvR_rw_init_mem(&rw, mem, size, size);

   //version
   RvR_rw_write_u16(&rw,0);

   //wall count
   RvR_rw_write_u32(&rw,map->wall_count);

   //sector count
   RvR_rw_write_u32(&rw,map->sector_count);

   //Walls
   for(int i = 0;i<map->wall_count;i++)
   {
      RvR_rw_write_u32(&rw,map->walls[i].x);
      RvR_rw_write_u32(&rw,map->walls[i].y);
      RvR_rw_write_u32(&rw,map->walls[i].flags);
      RvR_rw_write_u16(&rw,map->walls[i].p2);
      RvR_rw_write_u16(&rw,map->walls[i].portal);
      RvR_rw_write_u16(&rw,map->walls[i].join);
      RvR_rw_write_u16(&rw,map->walls[i].tex_lower);
      RvR_rw_write_u16(&rw,map->walls[i].tex_upper);
      RvR_rw_write_u16(&rw,map->walls[i].tex_mid);
   }

   //Sectors
   for(int i = 0;i<map->sector_count;i++)
   {
      RvR_rw_write_u16(&rw,map->sectors[i].wall_count);
      RvR_rw_write_u16(&rw,map->sectors[i].wall_first);
      RvR_rw_write_u32(&rw,map->sectors[i].floor);
      RvR_rw_write_u32(&rw,map->sectors[i].ceiling);
      RvR_rw_write_u16(&rw,map->sectors[i].floor_tex);
      RvR_rw_write_u16(&rw,map->sectors[i].ceiling_tex);
      RvR_rw_write_u32(&rw,map->sectors[i].flags);
   }

   //Compress and write to disk
   RvR_rw in;
   RvR_rw out;
   RvR_rw_init_mem(&in, mem, size, size);
   RvR_rw_init_path(&out, path, "wb");
   RvR_crush_compress(&in, &out, 0);
   RvR_rw_close(&in);
   RvR_rw_close(&out);

   //Free temp buffer
   RvR_rw_close(&rw);
   RvR_free(mem);
}

RvR_port_map *RvR_port_map_load(uint16_t id)
{
   char tmp[64];
   sprintf(tmp, "MAP%05d", id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   mem_pak = RvR_lump_get(tmp, &size_in);
   RvR_mem_tag_set(mem_pak, RVR_MALLOC_STATIC);
   RvR_rw rw_decomp;
   RvR_rw_init_const_mem(&rw_decomp, mem_pak, size_in);
   mem_decomp = RvR_crush_decompress(&rw_decomp, &size_out);
   RvR_mem_tag_set(mem_decomp, RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);
   RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw, mem_decomp, size_out);
   RvR_port_map *map = RvR_port_map_load_rw(&rw);
   RvR_rw_close(&rw);

   RvR_mem_tag_set(mem_decomp, RVR_MALLOC_CACHE);

   return map;
}

RvR_port_map *RvR_port_map_load_path(const char *path)
{
   RvR_error_check(path!=NULL, "RvR_port_map_load_path", "argument 'path' must be non-NULL\n");

   int32_t size = 0;
   RvR_rw rw_decomp;
   RvR_rw_init_path(&rw_decomp, path, "rb");
   uint8_t *mem = RvR_crush_decompress(&rw_decomp, &size);
   RvR_mem_tag_set(mem, RVR_MALLOC_STATIC);
   RvR_rw_close(&rw_decomp);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw, mem, size);
   RvR_port_map *map = RvR_port_map_load_rw(&rw);
   RvR_rw_close(&rw);

   RvR_mem_tag_set(mem, RVR_MALLOC_CACHE);

   return map;

RvR_err:
   return NULL;;
}

RvR_port_map *RvR_port_map_load_rw(RvR_rw *rw)
{
   //Read and check version
   uint16_t version = RvR_rw_read_u16(rw);
   RvR_error_check(version==0, "RvR_port_map_load_rw", "Invalid version '%d', expected version '0'\n", version);

   RvR_port_map *map = RvR_port_map_create();

   //wall and sector count
   map->wall_count = RvR_rw_read_u32(rw);
   map->sector_count = RvR_rw_read_u32(rw);
   map->walls = RvR_malloc(sizeof(*map->walls)*map->wall_count,"RvR_port map walls");
   map->sectors = RvR_malloc(sizeof(*map->sectors)*map->sector_count,"RvR_port map sectors");

   //Walls
   for(int i = 0;i<map->wall_count;i++)
   {
      map->walls[i].x = RvR_rw_read_u32(rw);
      map->walls[i].y = RvR_rw_read_u32(rw);
      map->walls[i].flags = RvR_rw_read_u32(rw);
      map->walls[i].p2 = RvR_rw_read_u16(rw);
      map->walls[i].portal = RvR_rw_read_u16(rw);
      map->walls[i].join = RvR_rw_read_u16(rw);
      map->walls[i].tex_lower = RvR_rw_read_u16(rw);
      map->walls[i].tex_upper = RvR_rw_read_u16(rw);
      map->walls[i].tex_mid = RvR_rw_read_u16(rw);
   }

   //Sectors
   for(int i = 0;i<map->sector_count;i++)
   {
      map->sectors[i].wall_count = RvR_rw_read_u16(rw);
      map->sectors[i].wall_first = RvR_rw_read_u16(rw);
      map->sectors[i].floor = RvR_rw_read_u32(rw);
      map->sectors[i].ceiling = RvR_rw_read_u32(rw);
      map->sectors[i].floor_tex = RvR_rw_read_u16(rw);
      map->sectors[i].ceiling_tex = RvR_rw_read_u16(rw);
      map->sectors[i].flags = RvR_rw_read_u32(rw);
   }

   return map;

RvR_err:
   return NULL;
}

int RvR_port_map_check(const RvR_port_map *map)
{
   //Check if walls sorted, sectors don't overlap, no gaps between sectors
   {
      int16_t next_first = 0;
      int16_t last_first = -1;
      for(int i = 0;i<map->sector_count;i++)
      {
         RvR_error_check(next_first>last_first,"RvR_port_map_check","walls of sector %d not sorted in relation to sector %d\n",i-1,i);
         RvR_error_check(map->sectors[i].wall_first>=next_first,"RvR_port_map_check","walls of sector %d and sector %d are overlapping\n",i-1,i);
         RvR_error_check(map->sectors[i].wall_first<=next_first,"RvR_port_map_check","gaps between walls of sector %d and sector %d\n",i-1,i);
         last_first = next_first;
         next_first+=map->sectors[i].wall_count;
      }
   }

   return 1;

RvR_err:
   return 0;
}
//-------------------------------------
