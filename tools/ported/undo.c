/*
RvnicRaven retro game engine

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "undo.h"
#include "map.h"
//-------------------------------------

//#defines
#define WRAP(p) ((p) & (UNDO_BUFFER_SIZE - 1))
#define UNDO_RECORD (UINT8_MAX)
#define REDO_RECORD (UINT8_MAX - 2)
#define JUNK_RECORD (UINT8_MAX - 1)

#define undo_read8(var,pos) do { var = undo_buffer[pos]; pos =  WRAP(pos-1); } while(0)
#define undo_read16(var,pos) do { uint16_t lo16,hi16; undo_read8(hi16,pos); undo_read8(lo16,pos); var = (uint16_t)(hi16*0x100+lo16); } while(0)
#define undo_read32(var,pos) do { uint32_t lo32,hi32; undo_read16(hi32,pos); undo_read16(lo32,pos); var = (uint32_t)(hi32*0x10000+lo32); } while(0)
#define undo_read64(var,pos) do { uint64_t lo64,hi64; undo_read32(hi64,pos); undo_read32(lo64,pos); var = (uint64_t)(hi64*0x100000000+lo64); } while(0)

#define redo_read8(var,pos) do { var = undo_buffer[pos]; pos = WRAP(pos+1); } while(0)
#define redo_read16(var,pos) do { uint16_t lo16,hi16; redo_read8(hi16,pos); redo_read8(lo16,pos); var = (uint16_t)(hi16*0x100+lo16); } while(0)
#define redo_read32(var,pos) do { uint32_t lo32,hi32; redo_read16(hi32,pos); redo_read16(lo32,pos); var = (uint32_t)(hi32*0x10000+lo32); } while(0)
#define redo_read64(var,pos) do { uint64_t lo64,hi64; redo_read32(hi64,pos); redo_read32(lo64,pos); var = (uint64_t)(hi64*0x100000000+lo64); } while(0)
//-------------------------------------

//Typedefs
typedef enum
{
   ED_WALL_SHADE,
   ED_FLOOR_SHADE,
   ED_CEILING_SHADE,
   ED_SECTOR_FLAG,
   ED_SPRITE_FLAG,
   ED_SPRITE_DIR,
   ED_WALL_FLAG,
   ED_WALL_UNITS,
   ED_SECTOR_UNITS,
   ED_SPRITE_UNITS,
   ED_WALL_OFFSETS,
   ED_SECTOR_OFFSETS,
   ED_SECTOR_SLOPE,
   ED_SECTOR_HEIGHT,
   ED_SPRITE_POS,
   ED_SPRITE_TEX,
   ED_WALL_TEX,
   ED_SECTOR_TEX,
}Ed_action;
//-------------------------------------

//Variables
static uint8_t *undo_buffer = NULL;
static int undo_len = 0;
static int undo_pos = 0;
static int redo_len = 0;
static uint32_t undo_entry_len = 0;
//-------------------------------------

//Function prototypes
static void undo_write8(uint8_t val);
static void undo_write16(uint16_t val);
static void undo_write32(uint32_t val);
static void undo_write64(uint64_t val);

static void undo_begin(Ed_action action);
static void undo_end();

static void redo_write8(uint8_t val);
static void redo_write16(uint16_t val);
static void redo_write32(uint32_t val);
static void redo_write64(uint64_t val);


static void undo_shade_wall(int pos, int endpos);
static void redo_shade_wall(int pos, int endpos);
static void undo_shade_floor(int pos, int endpos);
static void redo_shade_floor(int pos, int endpos);
static void undo_shade_ceiling(int pos, int endpos);
static void redo_shade_ceiling(int pos, int endpos);
static void undo_sector_flag(int pos, int endpos);
static void redo_sector_flag(int pos, int endpos);
static void undo_sprite_flag(int pos, int endpos);
static void redo_sprite_flag(int pos, int endpos);
static void undo_sprite_dir(int pos, int endpos);
static void redo_sprite_dir(int pos, int endpos);
static void undo_wall_flag(int pos, int endpos);
static void redo_wall_flag(int pos, int endpos);
static void undo_wall_units(int pos, int endpos);
static void redo_wall_units(int pos, int endpos);
static void undo_sector_units(int pos, int endpos);
static void redo_sector_units(int pos, int endpos);
static void undo_sprite_units(int pos, int endpos);
static void redo_sprite_units(int pos, int endpos);
static void undo_wall_offsets(int pos, int endpos);
static void redo_wall_offsets(int pos, int endpos);
static void undo_sector_offsets(int pos, int endpos);
static void redo_sector_offsets(int pos, int endpos);
static void undo_sector_slope(int pos, int endpos);
static void redo_sector_slope(int pos, int endpos);
static void undo_sector_height(int pos, int endpos);
static void redo_sector_height(int pos, int endpos);
static void undo_sprite_pos(int pos, int endpos);
static void redo_sprite_pos(int pos, int endpos);
static void undo_sprite_tex(int pos, int endpos);
static void redo_sprite_tex(int pos, int endpos);
static void undo_wall_tex(int pos, int endpos);
static void redo_wall_tex(int pos, int endpos);
static void undo_sector_tex(int pos, int endpos);
static void redo_sector_tex(int pos, int endpos);
//-------------------------------------

//Function implementations

void undo_init(void)
{
   undo_buffer = RvR_malloc(sizeof(*undo_buffer) * UNDO_BUFFER_SIZE, "ported undo buffer");
   memset(undo_buffer, 0, sizeof(*undo_buffer) * UNDO_BUFFER_SIZE);
}

void undo_reset(void)
{
   undo_len = 0;
   undo_pos = 0;
   redo_len = 0;
}

void undo(void)
{
   int pos = undo_pos;
   uint32_t len = 0;
   Ed_action action;

   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return;
   pos = WRAP(pos-1);
   undo_read32(len,pos);
   int endpos = WRAP(pos-len);
   //if(endpos<0)
      //return;
   undo_read16(action,endpos);
   endpos = WRAP(endpos+2);
   //pos = WRAP(undo_pos-6);
   //printf("%d %d\n",undo_pos,pos);

   //int endpos = undo_find_end(&len);
   //if(endpos<0)
      //return;
   //Ed_action action = undo_buffer[endpos];

   //endpos = WRAP(endpos);
   //pos = WRAP(undo_pos - 3);

   if(pos==endpos)
      return;


   //printf("len %d\n",len);

   //New redo entry
   redo_write8(REDO_RECORD);
   redo_write16(action);

   //Apply undoes
   switch(action)
   {
   case ED_WALL_SHADE: undo_shade_wall(pos,endpos); break;
   case ED_FLOOR_SHADE: undo_shade_floor(pos,endpos); break;
   case ED_CEILING_SHADE: undo_shade_ceiling(pos,endpos); break;
   case ED_SECTOR_FLAG: undo_sector_flag(pos,endpos); break;
   case ED_SPRITE_FLAG: undo_sprite_flag(pos,endpos); break;
   case ED_SPRITE_DIR: undo_sprite_dir(pos,endpos); break;
   case ED_WALL_FLAG: undo_wall_flag(pos,endpos); break;
   case ED_WALL_UNITS: undo_wall_units(pos,endpos); break;
   case ED_SECTOR_UNITS: undo_sector_units(pos,endpos); break;
   case ED_SPRITE_UNITS: undo_sprite_units(pos,endpos); break;
   case ED_WALL_OFFSETS: undo_wall_offsets(pos,endpos); break;
   case ED_SECTOR_OFFSETS: undo_sector_offsets(pos,endpos); break;
   case ED_SECTOR_SLOPE: undo_sector_slope(pos,endpos); break;
   case ED_SECTOR_HEIGHT: undo_sector_height(pos,endpos); break;
   case ED_SPRITE_POS: undo_sprite_pos(pos,endpos); break;
   case ED_SPRITE_TEX: undo_sprite_tex(pos,endpos); break;
   case ED_WALL_TEX: undo_wall_tex(pos,endpos); break;
   case ED_SECTOR_TEX: undo_sector_tex(pos,endpos); break;
   }

   redo_write32(len);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

void redo(void)
{
   int pos = undo_pos;
   uint32_t len = 0;
   Ed_action action;

   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return;
   if(redo_len<=0)
      return;
   pos = WRAP(pos+1);
   redo_read32(len,pos);
   int endpos = WRAP(pos+len);
   redo_read16(action,endpos);
   endpos = WRAP(endpos-2);
   //int endpos = redo_find_end(&len);
   //if(endpos<0)
      //return;

   //Ed_action action = undo_buffer[endpos];
   //pos = WRAP(undo_pos + 3);

   if(pos==endpos)
      return;

   //New undo entry
   undo_write8(UNDO_RECORD);
   undo_write16(action);

   //Apply redoes
   switch(action)
   {
   case ED_WALL_SHADE: redo_shade_wall(pos,endpos); break;
   case ED_FLOOR_SHADE: redo_shade_floor(pos,endpos); break;
   case ED_CEILING_SHADE: redo_shade_ceiling(pos,endpos); break;
   case ED_SECTOR_FLAG: redo_sector_flag(pos,endpos); break;
   case ED_SPRITE_FLAG: redo_sprite_flag(pos,endpos); break;
   case ED_SPRITE_DIR: redo_sprite_dir(pos,endpos); break;
   case ED_WALL_FLAG: redo_wall_flag(pos,endpos); break;
   case ED_WALL_UNITS: redo_wall_units(pos,endpos); break;
   case ED_SECTOR_UNITS: redo_sector_units(pos,endpos); break;
   case ED_SPRITE_UNITS: redo_sprite_units(pos,endpos); break;
   case ED_WALL_OFFSETS: redo_wall_offsets(pos,endpos); break;
   case ED_SECTOR_OFFSETS: redo_sector_offsets(pos,endpos); break;
   case ED_SECTOR_SLOPE: redo_sector_slope(pos,endpos); break;
   case ED_SECTOR_HEIGHT: redo_sector_height(pos,endpos); break;
   case ED_SPRITE_POS: redo_sprite_pos(pos,endpos); break;
   case ED_SPRITE_TEX: redo_sprite_tex(pos,endpos); break;
   case ED_WALL_TEX: redo_wall_tex(pos,endpos); break;
   case ED_SECTOR_TEX: redo_sector_tex(pos,endpos); break;
   }

   undo_write32(len);
   //undo_write16((len >> 16) & UINT16_MAX);
   //undo_write16(len & UINT16_MAX);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

static void undo_write8(uint8_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos + 1);
   undo_len += (undo_len<UNDO_BUFFER_SIZE - 2);
   redo_len -= (redo_len>0);
   undo_entry_len++;
}

static void undo_write16(uint16_t val)
{
   undo_write8((uint8_t)(val&255));
   undo_write8((uint8_t)((val>>8)&255));
}

static void undo_write32(uint32_t val)
{
   undo_write16((uint16_t)(val&0xffff));
   undo_write16((uint16_t)((val>>16)&0xffff));
}

static void undo_write64(uint64_t val)
{
   undo_write32((uint32_t)(val&0xffffffff));
   undo_write32((uint32_t)((val>>32)&0xffffffff));
}

static void redo_write8(uint8_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos - 1);
   redo_len += (redo_len<UNDO_BUFFER_SIZE - 2);
   undo_len -= (undo_len>0);
}

static void redo_write16(uint16_t val)
{
   redo_write8((uint8_t)(val&255));
   redo_write8((uint8_t)((val>>8)&255));
}

static void redo_write32(uint32_t val)
{
   redo_write16((uint16_t)(val&0xffff));
   redo_write16((uint16_t)((val>>16)&0xffff));
}

static void redo_write64(uint64_t val)
{
   redo_write32((uint32_t)(val&0xffffffff));
   redo_write32((uint32_t)((val>>32)&0xffffffff));
}

static void undo_begin(Ed_action action)
{
   redo_len = 0;
   undo_write8(UNDO_RECORD);
   undo_write16(action);
   undo_entry_len = 0;
}

static void undo_end()
{
   int pos = WRAP(undo_pos - 1);
   if(undo_len==0)
   {
      //empty
      undo_pos = pos;
      undo_len -= 1;
   }
   else
   {
      undo_write32(undo_entry_len);
   }
   undo_buffer[undo_pos] = JUNK_RECORD;
}

void undo_track_wall_shade(uint32_t wall)
{
   undo_begin(ED_WALL_SHADE);
   undo_write32(wall);
   undo_write8(map->walls[wall].shade_offset);
   undo_end();
}

static void undo_shade_wall(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      int8_t shade;
      undo_read8(shade,pos);
      undo_read32(wall,pos);

      redo_write32(wall);
      redo_write8(map->walls[wall].shade_offset);

      map->walls[wall].shade_offset = shade;
   }
}

static void redo_shade_wall(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      int8_t shade;
      redo_read8(shade,pos);
      redo_read32(wall,pos);

      undo_write32(wall);
      undo_write8(map->walls[wall].shade_offset);

      map->walls[wall].shade_offset = shade;
   }
}

void undo_track_floor_shade(uint32_t sector)
{
   undo_begin(ED_FLOOR_SHADE);
   undo_write32(sector);
   undo_write8(map->sectors[sector].shade_floor);
   undo_end();
}

static void undo_shade_floor(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      int8_t shade;
      undo_read8(shade,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write8(map->sectors[sector].shade_floor);

      map->sectors[sector].shade_floor= shade;
   }
}

static void redo_shade_floor(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      int8_t shade;
      redo_read8(shade,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write8(map->sectors[sector].shade_floor);

      map->sectors[sector].shade_floor = shade;
   }
}

void undo_track_ceiling_shade(uint32_t sector)
{
   undo_begin(ED_CEILING_SHADE);
   undo_write32(sector);
   undo_write8(map->sectors[sector].shade_ceiling);
   undo_end();
}

static void undo_shade_ceiling(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      int8_t shade;
      undo_read8(shade,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write8(map->sectors[sector].shade_ceiling);

      map->sectors[sector].shade_ceiling = shade;
   }
}

static void redo_shade_ceiling(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      int8_t shade;
      redo_read8(shade,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write8(map->sectors[sector].shade_ceiling);

      map->sectors[sector].shade_ceiling = shade;
   }
}

void undo_track_sector_flag(uint32_t sector)
{
   undo_begin(ED_SECTOR_FLAG);
   undo_write32(sector);
   undo_write32(map->sectors[sector].flags);
   undo_end();
}

static void undo_sector_flag(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint32_t flag;
      undo_read32(flag,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write32(map->sectors[sector].flags);

      map->sectors[sector].flags = flag;
   }
}

static void redo_sector_flag(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint32_t flag;
      redo_read32(flag,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write32(map->sectors[sector].flags);

      map->sectors[sector].flags = flag;
   }
}

void undo_track_sprite_flag(uint32_t sprite)
{
   undo_begin(ED_SPRITE_FLAG);
   undo_write32(sprite);
   undo_write32(map->sprites[sprite].flags);
   undo_end();
}

static void undo_sprite_flag(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint32_t flag;
      undo_read32(flag,pos);
      undo_read32(sprite,pos);

      redo_write32(sprite);
      redo_write32(map->sprites[sprite].flags);

      map->sprites[sprite].flags = flag;
   }
}

static void redo_sprite_flag(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint32_t flag;
      redo_read32(flag,pos);
      redo_read32(sprite,pos);

      undo_write32(sprite);
      undo_write32(map->sprites[sprite].flags);

      map->sprites[sprite].flags = flag;
   }
}

void undo_track_sprite_dir(uint32_t sprite)
{
   undo_begin(ED_SPRITE_DIR);
   undo_write32(sprite);
   undo_write32(map->sprites[sprite].dir);
   undo_end();
}

static void undo_sprite_dir(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint32_t dir;
      undo_read32(dir,pos);
      undo_read32(sprite,pos);

      redo_write32(sprite);
      redo_write32(map->sprites[sprite].dir);

      map->sprites[sprite].dir = dir;
   }
}

static void redo_sprite_dir(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint32_t dir;
      redo_read32(dir,pos);
      redo_read32(sprite,pos);

      undo_write32(sprite);
      undo_write32(map->sprites[sprite].dir);

      map->sprites[sprite].dir = dir;
   }
}

void undo_track_wall_flag(uint32_t wall)
{
   undo_begin(ED_WALL_FLAG);
   undo_write32(wall);
   undo_write32(map->walls[wall].flags);
   undo_end();
}

static void undo_wall_flag(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint32_t flag;
      undo_read32(flag,pos);
      undo_read32(wall,pos);

      redo_write32(wall);
      redo_write32(map->walls[wall].flags);

      map->walls[wall].flags = flag;
   }
}

static void redo_wall_flag(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint32_t flag;
      redo_read32(flag,pos);
      redo_read32(wall,pos);

      undo_write32(wall);
      undo_write32(map->walls[wall].flags);

      map->walls[wall].flags = flag;
   }
}

void undo_track_wall_units(uint32_t wall)
{
   undo_begin(ED_WALL_UNITS);
   undo_write32(wall);
   undo_write8(map->walls[wall].x_units);
   undo_write8(map->walls[wall].y_units);
   undo_end();
}

static void undo_wall_units(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint8_t xunits;
      uint8_t yunits;
      undo_read8(yunits,pos);
      undo_read8(xunits,pos);
      undo_read32(wall,pos);

      redo_write32(wall);
      redo_write8(map->walls[wall].x_units);
      redo_write8(map->walls[wall].y_units);

      map->walls[wall].x_units = xunits;
      map->walls[wall].y_units = yunits;
   }
}

static void redo_wall_units(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint8_t xunits;
      uint8_t yunits;
      redo_read8(yunits,pos);
      redo_read8(xunits,pos);
      redo_read32(wall,pos);

      undo_write32(wall);
      undo_write8(map->walls[wall].x_units);
      undo_write8(map->walls[wall].y_units);

      map->walls[wall].x_units = xunits;
      map->walls[wall].y_units = yunits;
   }
}

void undo_track_sector_units(uint32_t sector)
{
   undo_begin(ED_SECTOR_UNITS);
   undo_write32(sector);
   undo_write8(map->sectors[sector].x_units);
   undo_write8(map->sectors[sector].y_units);
   undo_end();
}

static void undo_sector_units(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint8_t xunits;
      uint8_t yunits;
      undo_read8(yunits,pos);
      undo_read8(xunits,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write8(map->sectors[sector].x_units);
      redo_write8(map->sectors[sector].y_units);

      map->sectors[sector].x_units = xunits;
      map->sectors[sector].y_units = yunits;
   }
}

static void redo_sector_units(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint8_t xunits;
      uint8_t yunits;
      redo_read8(yunits,pos);
      redo_read8(xunits,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write8(map->sectors[sector].x_units);
      undo_write8(map->sectors[sector].y_units);

      map->sectors[sector].x_units = xunits;
      map->sectors[sector].y_units = yunits;
   }
}

void undo_track_sprite_units(uint32_t sprite)
{
   undo_begin(ED_SPRITE_UNITS);
   undo_write32(sprite);
   undo_write8(map->sprites[sprite].x_units);
   undo_write8(map->sprites[sprite].y_units);
   undo_end();
}

static void undo_sprite_units(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint8_t xunits;
      uint8_t yunits;
      undo_read8(yunits,pos);
      undo_read8(xunits,pos);
      undo_read32(sprite,pos);

      redo_write32(sprite);
      redo_write8(map->sprites[sprite].x_units);
      redo_write8(map->sprites[sprite].y_units);

      map->sprites[sprite].x_units = xunits;
      map->sprites[sprite].y_units = yunits;
   }
}

static void redo_sprite_units(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint8_t xunits;
      uint8_t yunits;
      redo_read8(yunits,pos);
      redo_read8(xunits,pos);
      redo_read32(sprite,pos);

      undo_write32(sprite);
      undo_write8(map->sprites[sprite].x_units);
      undo_write8(map->sprites[sprite].y_units);

      map->sprites[sprite].x_units = xunits;
      map->sprites[sprite].y_units = yunits;
   }
}

void undo_track_wall_offsets(uint32_t wall)
{
   undo_begin(ED_WALL_OFFSETS);
   undo_write32(wall);
   undo_write16(map->walls[wall].x_off);
   undo_write16(map->walls[wall].y_off);
   undo_end();
}

static void undo_wall_offsets(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint16_t xoff;
      uint16_t yoff;
      undo_read16(yoff,pos);
      undo_read16(xoff,pos);
      undo_read32(wall,pos);

      redo_write32(wall);
      redo_write16(map->walls[wall].x_off);
      redo_write16(map->walls[wall].y_off);

      map->walls[wall].x_off = (int16_t)xoff;
      map->walls[wall].y_off = (int16_t)yoff;
   }
}

static void redo_wall_offsets(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint16_t xoff;
      uint16_t yoff;
      redo_read16(yoff,pos);
      redo_read16(xoff,pos);
      redo_read32(wall,pos);

      undo_write32(wall);
      undo_write16(map->walls[wall].x_off);
      undo_write16(map->walls[wall].y_off);

      map->walls[wall].x_off = xoff;
      map->walls[wall].y_off = yoff;
   }
}

void undo_track_sector_offsets(uint32_t sector)
{
   undo_begin(ED_SECTOR_OFFSETS);
   undo_write32(sector);
   undo_write16(map->sectors[sector].x_off);
   undo_write16(map->sectors[sector].y_off);
   undo_end();
}

static void undo_sector_offsets(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint16_t xoff;
      uint16_t yoff;
      undo_read16(yoff,pos);
      undo_read16(xoff,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write16(map->sectors[sector].x_off);
      redo_write16(map->sectors[sector].y_off);

      map->sectors[sector].x_off = (int16_t)xoff;
      map->sectors[sector].y_off = (int16_t)yoff;
   }
}

static void redo_sector_offsets(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint16_t xoff;
      uint16_t yoff;
      redo_read16(yoff,pos);
      redo_read16(xoff,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write16(map->sectors[sector].x_off);
      undo_write16(map->sectors[sector].y_off);

      map->sectors[sector].x_off = xoff;
      map->sectors[sector].y_off = yoff;
   }
}

void undo_track_sector_slope(uint32_t sector)
{
   undo_begin(ED_SECTOR_SLOPE);
   undo_write32(sector);
   undo_write16(map->sectors[sector].slope_floor);
   undo_write16(map->sectors[sector].slope_ceiling);
   undo_end();
}

static void undo_sector_slope(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint16_t slope_floor;
      uint16_t slope_ceiling;
      undo_read16(slope_ceiling,pos);
      undo_read16(slope_floor,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write16(map->sectors[sector].slope_floor);
      redo_write16(map->sectors[sector].slope_ceiling);

      map->sectors[sector].slope_floor = (int16_t)slope_floor;
      map->sectors[sector].slope_ceiling = (int16_t)slope_ceiling;
   }
}

static void redo_sector_slope(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint16_t slope_floor;
      uint16_t slope_ceiling;
      redo_read16(slope_ceiling,pos);
      redo_read16(slope_floor,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write16(map->sectors[sector].slope_floor);
      undo_write16(map->sectors[sector].slope_ceiling);

      map->sectors[sector].slope_floor = slope_floor;
      map->sectors[sector].slope_ceiling = slope_ceiling;
   }
}

void undo_track_sector_height(uint32_t sector)
{
   undo_begin(ED_SECTOR_HEIGHT);
   undo_write32(sector);
   undo_write32(map->sectors[sector].floor);
   undo_write32(map->sectors[sector].ceiling);
   printf("%d %d\n",map->sectors[sector].floor,map->sectors[sector].ceiling);
   undo_end();
}

static void undo_sector_height(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint32_t floor;
      uint32_t ceiling;
      undo_read32(ceiling,pos);
      undo_read32(floor,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write32(map->sectors[sector].floor);
      redo_write32(map->sectors[sector].ceiling);

      map->sectors[sector].floor = floor;
      map->sectors[sector].ceiling = ceiling;
   }
}

static void redo_sector_height(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint32_t floor;
      uint32_t ceiling;
      redo_read32(ceiling,pos);
      redo_read32(floor,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write32(map->sectors[sector].floor);
      undo_write32(map->sectors[sector].ceiling);

      map->sectors[sector].floor = floor;
      map->sectors[sector].ceiling = ceiling;
   }
}

void undo_track_sprite_pos(uint32_t sprite)
{
   undo_begin(ED_SPRITE_POS);
   undo_write32(sprite);
   undo_write32(map->sprites[sprite].x);
   undo_write32(map->sprites[sprite].y);
   undo_write32(map->sprites[sprite].z);
   undo_write32(map->sprites[sprite].sector);
   undo_end();
}

static void undo_sprite_pos(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint32_t x;
      uint32_t y;
      uint32_t z;
      uint32_t sector;
      undo_read32(sector,pos);
      undo_read32(z,pos);
      undo_read32(y,pos);
      undo_read32(x,pos);
      undo_read32(sprite,pos);

      redo_write32(sprite);
      redo_write32(map->sprites[sprite].x);
      redo_write32(map->sprites[sprite].y);
      redo_write32(map->sprites[sprite].z);
      redo_write32(map->sprites[sprite].sector);

      map->sprites[sprite].x = x;
      map->sprites[sprite].y = y;
      map->sprites[sprite].z = z;
      map->sprites[sprite].sector = sector;
   }
}

static void redo_sprite_pos(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint32_t x;
      uint32_t y;
      uint32_t z;
      uint32_t sector;
      redo_read32(sector,pos);
      redo_read32(z,pos);
      redo_read32(y,pos);
      redo_read32(x,pos);
      redo_read32(sprite,pos);

      undo_write32(sprite);
      undo_write32(map->sprites[sprite].x);
      undo_write32(map->sprites[sprite].y);
      undo_write32(map->sprites[sprite].z);
      undo_write32(map->sprites[sprite].sector);

      map->sprites[sprite].x = x;
      map->sprites[sprite].y = y;
      map->sprites[sprite].z = z;
      map->sprites[sprite].sector = sector;
   }
}

void undo_track_sprite_tex(uint32_t sprite)
{
   undo_begin(ED_SPRITE_TEX);
   undo_write32(sprite);
   undo_write16(map->sprites[sprite].tex);
   undo_end();
}

static void undo_sprite_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint16_t tex;
      undo_read16(tex,pos);
      undo_read32(sprite,pos);

      redo_write32(sprite);
      redo_write16(map->sprites[sprite].tex);

      map->sprites[sprite].tex = tex;
   }
}

static void redo_sprite_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint16_t tex;
      redo_read16(tex,pos);
      redo_read32(sprite,pos);

      undo_write32(sprite);
      undo_write16(map->sprites[sprite].tex);

      map->sprites[sprite].tex = tex;
   }
}

void undo_track_wall_tex(uint32_t wall)
{
   undo_begin(ED_WALL_TEX);
   undo_write32(wall);
   undo_write16(map->walls[wall].tex_upper);
   undo_write16(map->walls[wall].tex_lower);
   undo_write16(map->walls[wall].tex_mid);
   undo_end();
}

static void undo_wall_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint16_t tex_upper;
      uint16_t tex_lower;
      uint16_t tex_mid;
      undo_read16(tex_mid,pos);
      undo_read16(tex_lower,pos);
      undo_read16(tex_upper,pos);
      undo_read32(wall,pos);

      redo_write32(wall);
      redo_write16(map->walls[wall].tex_upper);
      redo_write16(map->walls[wall].tex_lower);
      redo_write16(map->walls[wall].tex_mid);

      map->walls[wall].tex_upper = tex_upper;
      map->walls[wall].tex_lower = tex_lower;
      map->walls[wall].tex_mid = tex_mid;
   }
}

static void redo_wall_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint16_t tex_upper;
      uint16_t tex_lower;
      uint16_t tex_mid;
      redo_read16(tex_mid,pos);
      redo_read16(tex_lower,pos);
      redo_read16(tex_upper,pos);
      redo_read32(wall,pos);

      undo_write32(wall);
      undo_write16(map->walls[wall].tex_upper);
      undo_write16(map->walls[wall].tex_lower);
      undo_write16(map->walls[wall].tex_mid);

      map->walls[wall].tex_upper = tex_upper;
      map->walls[wall].tex_lower = tex_lower;
      map->walls[wall].tex_mid = tex_mid;
   }
}

void undo_track_sector_tex(uint32_t sector)
{
   undo_begin(ED_SECTOR_TEX);
   undo_write32(sector);
   undo_write16(map->sectors[sector].floor_tex);
   undo_write16(map->sectors[sector].ceiling_tex);
   undo_end();
}

static void undo_sector_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint16_t tex_floor;
      uint16_t tex_ceiling;
      undo_read16(tex_ceiling,pos);
      undo_read16(tex_floor,pos);
      undo_read32(sector,pos);

      redo_write32(sector);
      redo_write16(map->sectors[sector].floor_tex);
      redo_write16(map->sectors[sector].ceiling_tex);

      map->sectors[sector].floor_tex = tex_floor;
      map->sectors[sector].ceiling_tex = tex_ceiling;
   }
}

static void redo_sector_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      uint16_t tex_floor;
      uint16_t tex_ceiling;
      redo_read16(tex_ceiling,pos);
      redo_read16(tex_floor,pos);
      redo_read32(sector,pos);

      undo_write32(sector);
      undo_write16(map->sectors[sector].floor_tex);
      undo_write16(map->sectors[sector].ceiling_tex);

      map->sectors[sector].floor_tex = tex_floor;
      map->sectors[sector].ceiling_tex = tex_ceiling;
   }
}
//-------------------------------------
