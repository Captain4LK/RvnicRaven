/*
RvnicRaven-portal

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
   size_t size = 0;
   size += 2; //version
   size += 2; //map->wall_count
   size += 2; //map->sector_count
   size+=map->wall_count*(4+4+4+2+2+2+2+2+2+1+2+2+1+1); //Walls
   size+=map->sector_count*(2+2+4+4+2+2+4+2+2+1+1+2+2); //Sectors
   size+=map->sprite_count*(4+4+4+4+2+2+4); //Sprites

   uint8_t *mem = RvR_malloc(size, "RvR_port map save buffer");
   RvR_rw rw = {0};
   RvR_rw_init_mem(&rw, mem, size, size);

   //version
   RvR_rw_write_u16(&rw,0);

   //wall count
   RvR_rw_write_u16(&rw,map->wall_count);

   //sector count
   RvR_rw_write_u16(&rw,map->sector_count);

   //sprite count
   RvR_rw_write_u16(&rw,map->sprite_count);

   //Walls
   for(int i = 0;i<map->wall_count;i++)
   {
      RvR_rw_write_u32(&rw,map->walls[i].x);
      RvR_rw_write_u32(&rw,map->walls[i].y);
      RvR_rw_write_u32(&rw,map->walls[i].flags);
      RvR_rw_write_u16(&rw,map->walls[i].p2);
      RvR_rw_write_u16(&rw,map->walls[i].portal);
      RvR_rw_write_u16(&rw,map->walls[i].portal_wall);
      RvR_rw_write_u16(&rw,map->walls[i].tex_lower);
      RvR_rw_write_u16(&rw,map->walls[i].tex_upper);
      RvR_rw_write_u16(&rw,map->walls[i].tex_mid);
      RvR_rw_write_u8(&rw,map->walls[i].shade_offset);
      RvR_rw_write_u16(&rw,map->walls[i].x_off);
      RvR_rw_write_u16(&rw,map->walls[i].y_off);
      RvR_rw_write_u8(&rw,map->walls[i].x_units);
      RvR_rw_write_u8(&rw,map->walls[i].y_units);
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
      RvR_rw_write_u16(&rw,map->sectors[i].x_off);
      RvR_rw_write_u16(&rw,map->sectors[i].y_off);
      RvR_rw_write_u8(&rw,map->sectors[i].shade_floor);
      RvR_rw_write_u8(&rw,map->sectors[i].shade_ceiling);
      RvR_rw_write_u16(&rw,map->sectors[i].slope_floor);
      RvR_rw_write_u16(&rw,map->sectors[i].slope_ceiling);
   }

   //Sprites
   for(int i = 0;i<map->sprite_count;i++)
   {
      RvR_rw_write_u32(&rw,map->sprites[i].x);
      RvR_rw_write_u32(&rw,map->sprites[i].y);
      RvR_rw_write_u32(&rw,map->sprites[i].z);
      RvR_rw_write_u32(&rw,map->sprites[i].dir);
      RvR_rw_write_u16(&rw,map->sprites[i].sector);
      RvR_rw_write_u16(&rw,map->sprites[i].tex);
      RvR_rw_write_u32(&rw,map->sprites[i].flags);
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
   map->wall_count = RvR_rw_read_u16(rw);
   map->sector_count = RvR_rw_read_u16(rw);
   map->sprite_count = RvR_rw_read_u16(rw);
   map->walls = RvR_malloc(sizeof(*map->walls)*map->wall_count,"RvR_port map walls");
   map->sectors = RvR_malloc(sizeof(*map->sectors)*map->sector_count,"RvR_port map sectors");
   map->sprites = RvR_malloc(sizeof(*map->sprites)*map->sprite_count,"RvR_port map sprites");

   //Walls
   for(int i = 0;i<map->wall_count;i++)
   {
      map->walls[i].x = RvR_rw_read_u32(rw);
      map->walls[i].y = RvR_rw_read_u32(rw);
      map->walls[i].flags = RvR_rw_read_u32(rw);
      map->walls[i].p2 = RvR_rw_read_u16(rw);
      map->walls[i].portal = RvR_rw_read_u16(rw);
      map->walls[i].portal_wall = RvR_rw_read_u16(rw);
      map->walls[i].tex_lower = RvR_rw_read_u16(rw);
      map->walls[i].tex_upper = RvR_rw_read_u16(rw);
      map->walls[i].tex_mid = RvR_rw_read_u16(rw);
      map->walls[i].shade_offset = RvR_rw_read_u8(rw);
      map->walls[i].x_off = RvR_rw_read_u16(rw);
      map->walls[i].y_off = RvR_rw_read_u16(rw);
      map->walls[i].x_units = RvR_rw_read_u8(rw);
      map->walls[i].y_units = RvR_rw_read_u8(rw);
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
      map->sectors[i].x_off = RvR_rw_read_u16(rw);
      map->sectors[i].y_off = RvR_rw_read_u16(rw);
      map->sectors[i].shade_floor = RvR_rw_read_u8(rw);
      map->sectors[i].shade_ceiling = RvR_rw_read_u8(rw);
      map->sectors[i].slope_floor = RvR_rw_read_u16(rw);
      map->sectors[i].slope_ceiling = RvR_rw_read_u16(rw);
   }

   for(int i = 0;i<map->sprite_count;i++)
   {
      map->sprites[i].x = RvR_rw_read_u32(rw);
      map->sprites[i].y = RvR_rw_read_u32(rw);
      map->sprites[i].z = RvR_rw_read_u32(rw);
      map->sprites[i].dir = RvR_rw_read_u32(rw);
      map->sprites[i].sector = RvR_rw_read_u16(rw);
      map->sprites[i].tex = RvR_rw_read_u16(rw);
      map->sprites[i].flags = RvR_rw_read_u32(rw);
   }

   return map;

RvR_err:
   return NULL;
}

int RvR_port_map_check(const RvR_port_map *map)
{
   //Check if walls sorted, sectors don't overlap, no gaps between sectors
   {
      int32_t next_first = 0;
      int32_t last_first = -1;
      for(int i = 0;i<map->sector_count;i++)
      {
         RvR_error_check(next_first>last_first,"RvR_port_map_check","walls of sector %d not sorted in relation to sector %d\n",i-1,i);
         RvR_error_check(map->sectors[i].wall_first>=next_first,"RvR_port_map_check","walls of sector %d and sector %d are overlapping\n",i-1,i);
         RvR_error_check(map->sectors[i].wall_first<=next_first,"RvR_port_map_check","gaps between walls of sector %d and sector %d\n",i-1,i);
         last_first = next_first;
         next_first+=map->sectors[i].wall_count;
      }
   }

   //Check sectors
   for(int i = 0;i<map->sector_count;i++)
   {
      RvR_error_check(map->sectors[i].wall_first!=RVR_PORT_WALL_INVALID&&map->sectors[i].wall_first<map->wall_count,"RvR_port_map_check","wall_first %d for sector %d out of bounds\n",map->sectors[i].wall_first,i);
      RvR_error_check(map->sectors[i].wall_count>0,"RvR_port_map_check","wall_count %d for sector %d invalid\n",map->sectors[i].wall_count,i);
      RvR_error_check(map->sectors[i].wall_first+map->sectors[i].wall_count<=map->wall_count,"RvR_port_map_check","wall range (%d, %d) of sector %d out of bounds\n",map->sectors[i].wall_first,map->sectors[i].wall_first+map->sectors[i].wall_count-1,i);
   }

   //TODO(Captain4LK): check p2 of walls:
   //must be in sector
   //must be either wall+1, or first wall of subsector

   //Check portals
   for(int i = 0;i<map->wall_count;i++)
   {
      if(map->walls[i].portal!=RVR_PORT_SECTOR_INVALID)
      {
         //RvR_error_check(map->walls[i].portal>=0,"RvR_port_map_check","invalid portal value %d for wall %d\n",map->walls[i].portal,i);
         RvR_error_check(map->walls[i].portal<map->sector_count,"RvR_port_map_check","portal value %d of wall %d out of bounds\n",map->walls[i].portal,i);
         RvR_error_check(map->walls[i].portal_wall!=RVR_PORT_WALL_INVALID,"RvR_port_map_check","portal of wall %d is %d, but portal_wall not set\n",i,map->walls[i].portal);
      }

      if(map->walls[i].portal_wall==RVR_PORT_WALL_INVALID)
         continue;

      int16_t pwall = map->walls[i].portal_wall;

      //RvR_error_check(map->walls[i].portal_wall>=0,"RvR_port_map_check","invalid portal_wall value %d for wall %d\n",map->walls[i].portal_wall,i);
      RvR_error_check(map->walls[i].portal_wall<map->wall_count,"RvR_port_map_check","portal_wall value %d of wall %d out of bounds\n",map->walls[i].portal_wall,i);
      RvR_error_check(map->walls[pwall].portal_wall==i,"RvR_port_map_check","portal wall of wall %d not referencing wall %d\n",pwall,i);
      RvR_error_check(map->walls[i].portal!=RVR_PORT_SECTOR_INVALID,"RvR_port_map_check","portal_wall of wall %d is %d, but portal not set\n",i,pwall);

      RvR_fix22 x00 = map->walls[i].x;
      RvR_fix22 y00 = map->walls[i].y;
      RvR_fix22 x01 = map->walls[map->walls[i].p2].x;
      RvR_fix22 y01 = map->walls[map->walls[i].p2].y;
      RvR_fix22 x10 = map->walls[pwall].x;
      RvR_fix22 y10 = map->walls[pwall].y;
      RvR_fix22 x11 = map->walls[map->walls[pwall].p2].x;
      RvR_fix22 y11 = map->walls[map->walls[pwall].p2].y;

      RvR_error_check(x00==x11&&y00==y11&& x01==x10&&y01==y10, "RvR_port_map_check","wall %d and portal_wall %d don't match up\n",i,pwall);
   }

   //Check winding
   for(int i = 0;i<map->sector_count;i++)
   {
      int wall = 0;
      int subsector = 0;
      RvR_port_sector *s = map->sectors+i;
      while(wall<map->sectors[i].wall_count-1)
      {
         int64_t sum = 0;
         int first = wall;

         //Calculate winding number, positive if clockwise
         for(;wall==first||map->walls[map->sectors[i].wall_first+wall-1].p2==map->sectors[i].wall_first+wall;wall++)
         {
            int64_t x0 = map->walls[s->wall_first+wall].x;
            int64_t y0 = map->walls[s->wall_first+wall].y;
            int64_t x1 = map->walls[map->walls[s->wall_first+wall].p2].x;
            int64_t y1 = map->walls[map->walls[s->wall_first+wall].p2].y;
            sum+=(x1-x0)*(y0+y1);
         }

         //First subsector must be clockwise, all other subsectors counter-clockwise
         RvR_error_check(!((first==0&&sum>0)||(first!=0&&sum<0)),"RvR_port_map_check","winding of subsector %d of sector %d incorrect\n",subsector,i);

         subsector++;
      }
   }

   //Check for overlapping walls
   //Last because n^2
   for(int i = 0;i<map->wall_count;i++)
   {
      for(int j = i+1;j<map->wall_count;j++)
      {
         //Two sides of the same wall
         if(map->walls[i].portal_wall==j)
            continue;

         int64_t x0 = map->walls[i].x;
         int64_t y0 = map->walls[i].y;
         int64_t x1 = map->walls[map->walls[i].p2].x;
         int64_t y1 = map->walls[map->walls[i].p2].y;
         int64_t x2 = map->walls[j].x;
         int64_t y2 = map->walls[j].y;
         int64_t x3 = map->walls[map->walls[j].p2].x;
         int64_t y3 = map->walls[map->walls[j].p2].y;

         int64_t t = ((-(y1-y0)*(x0-x2)+(x1-x0)*(y0-y2))*65536)/RvR_non_zero(-(x3-x2)*(y1-y0)+(x1-x0)*(y3-y2));
         int64_t u = (((x3-x2)*(y0-y2)-(y3-y2)*(x0-x2))*65536)/RvR_non_zero(-(x3-x2)*(y1-y0)+(x1-x0)*(y3-y2));

         //If walls touch, the value of both will be 0 or 65536, but these are still valid
         if((t==0||t==65536)&&(u==0||u==65536))
            continue;

         RvR_error_check((t<0||t>65536)||(u<0||u>65536),"RvR_port_map_check","walls %d and %d intersect (%ld %ld) (%ld,%ld) (%ld,%ld) (%ld,%ld) (%ld,%ld)\n",i,j,t,u,x0,y0,x1,y1,x2,y2,x3,y3);
      }
   }

   return 1;

RvR_err:
   return 0;
}

void RvR_port_map_print_walls(const RvR_port_map *map)
{
   for(int i = 0;i<map->wall_count;i++)
   {
      printf("Wall %5d: (%d %d), portal = %d, portal_wall = %d, sector = %d, p2 = %d\n",i,map->walls[i].x,map->walls[i].y,map->walls[i].portal,map->walls[i].portal_wall,RvR_port_wall_sector(map,(uint16_t)i),map->walls[i].p2);
   }
}
//-------------------------------------
