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
#include <inttypes.h>

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

#define redo_read8(var,pos) do { var = redo_buffer[pos]; pos = WRAP(pos-1); } while(0)
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
   ED_SPRITE_DEL,
   ED_WALL_MOVE,
   ED_SECTOR_ADD,
   ED_SECTOR_ADD_INNER,
   ED_SECTOR_ADD_OVERLAP,
   ED_SECTOR_SPLIT,
   ED_SECTOR_CONNECT,
}Ed_action;
//-------------------------------------

//Variables
static uint8_t *undo_buffer = NULL;
static uint8_t *redo_buffer = NULL;
static int undo_len = 0;
static int undo_pos = 0;
static int redo_pos = 0;
static int redo_len = 0;
static uint32_t undo_entry_len = 0;
static uint32_t redo_entry_len = 0;
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
static void undo_sprite_del(int pos, int endpos);
static void redo_sprite_del(int pos, int endpos);
static void undo_wall_move(int pos, int endpos);
static void redo_wall_move(int pos, int endpos);
static void undo_sector_add(int pos, int endpos);
static void redo_sector_add(int pos, int endpos);
static void undo_sector_add_inner(int pos, int endpos);
static void redo_sector_add_inner(int pos, int endpos);
static void undo_sector_add_overlap(int pos, int endpos);
static void redo_sector_add_overlap(int pos, int endpos);
static void undo_sector_split(int pos, int endpos);
static void redo_sector_split(int pos, int endpos);
static void undo_sector_connect(int pos, int endpos);
static void redo_sector_connect(int pos, int endpos);
//-------------------------------------

//Function implementations

void undo_init(void)
{
   undo_buffer = RvR_malloc(sizeof(*undo_buffer) * UNDO_BUFFER_SIZE, "ported undo buffer");
   memset(undo_buffer, 0, sizeof(*undo_buffer) * UNDO_BUFFER_SIZE);
   redo_buffer = RvR_malloc(sizeof(*redo_buffer) * UNDO_BUFFER_SIZE, "ported redo buffer");
   memset(redo_buffer, 0, sizeof(*redo_buffer) * UNDO_BUFFER_SIZE);
}

void undo_reset(void)
{
   undo_len = 0;
   undo_pos = 0;
   redo_len = 0;
   redo_pos = 0;
}

void undo(void)
{
   int pos = undo_pos;
   uint32_t len = 0;
   Ed_action action;

   //printf("Start: %d\n",undo_pos);

   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return;
   //puts("UNDO");
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

   //printf("%d %d\n",pos,endpos);
   if(pos==endpos)
      return;


   //printf("len %d\n",len);

   //New redo entry
   redo_write8(REDO_RECORD);
   redo_write16(action);
   redo_entry_len = 0;


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
   case ED_SPRITE_DEL: undo_sprite_del(pos,endpos); break;
   case ED_WALL_MOVE: undo_wall_move(pos,endpos); break;
   case ED_SECTOR_ADD: undo_sector_add(pos,endpos); break;
   case ED_SECTOR_ADD_INNER: undo_sector_add_inner(pos,endpos); break;
   case ED_SECTOR_ADD_OVERLAP: undo_sector_add_overlap(pos,endpos); break;
   case ED_SECTOR_SPLIT: undo_sector_split(pos,endpos); break;
   case ED_SECTOR_CONNECT: undo_sector_connect(pos,endpos); break;
   }

   redo_write32(redo_entry_len);
   redo_buffer[redo_pos] = JUNK_RECORD;
   undo_pos = WRAP(undo_pos-len-7);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

void redo(void)
{
   int pos = redo_pos;
   uint32_t len = 0;
   Ed_action action;

   if(redo_buffer[redo_pos]!=JUNK_RECORD)
      return;
   if(redo_len<=0)
      return;
   pos = WRAP(pos-1);
   redo_read32(len,pos);
   int endpos = WRAP(pos-len);
   redo_read16(action,endpos);
   endpos = WRAP(endpos+2);
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
   undo_entry_len = 0;
   printf("%d\n",action);

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
   case ED_SPRITE_DEL: redo_sprite_del(pos,endpos); break;
   case ED_WALL_MOVE: redo_wall_move(pos,endpos); break;
   case ED_SECTOR_ADD: redo_sector_add(pos,endpos); break;
   case ED_SECTOR_ADD_INNER: redo_sector_add_inner(pos,endpos); break;
   case ED_SECTOR_ADD_OVERLAP: redo_sector_add_overlap(pos,endpos); break;
   case ED_SECTOR_SPLIT: redo_sector_split(pos,endpos); break;
   case ED_SECTOR_CONNECT: redo_sector_connect(pos,endpos); break;
   }

   undo_write32(undo_entry_len);
   //undo_write16((len >> 16) & UINT16_MAX);
   //undo_write16(len & UINT16_MAX);
   undo_buffer[undo_pos] = JUNK_RECORD;
   redo_pos = WRAP(redo_pos-len-7);
   redo_buffer[redo_pos] = JUNK_RECORD;
}

static void undo_write8(uint8_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos + 1);
   undo_len += (undo_len<UNDO_BUFFER_SIZE - 2);
   //redo_len -= (redo_len>0);
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
   int pos = redo_pos;
   redo_buffer[pos] = val;
   redo_pos = WRAP(pos + 1);
   redo_len += (redo_len<UNDO_BUFFER_SIZE - 2);
   //undo_len -= (undo_len>0);
   redo_entry_len++;

   //int pos = undo_pos;
   //undo_buffer[pos] = val;
   //undo_pos = WRAP(pos - 1);
   //redo_len += (redo_len<UNDO_BUFFER_SIZE - 2);
   //undo_len -= (undo_len>0);
   //redo_entry_len++;
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
   redo_pos = 0;
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

void undo_track_sprite_del(uint32_t sprite)
{
   undo_begin(ED_SPRITE_DEL);
   undo_write32(sprite);
   undo_write32(map->sprites[sprite].x);
   undo_write32(map->sprites[sprite].y);
   undo_write32(map->sprites[sprite].z);
   undo_write32(map->sprites[sprite].dir);
   undo_write16(map->sprites[sprite].sector);
   undo_write16(map->sprites[sprite].tex);
   undo_write32(map->sprites[sprite].flags);
   undo_write8(map->sprites[sprite].x_units);
   undo_write8(map->sprites[sprite].y_units);
   undo_end();
}

static void undo_sprite_del(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;
      uint32_t x;
      uint32_t y;
      uint32_t z;
      uint32_t dir;
      uint16_t sector;
      uint16_t tex;
      uint32_t flags;
      uint8_t x_units;
      uint8_t y_units;

      undo_read8(y_units,pos);
      undo_read8(x_units,pos);
      undo_read32(flags,pos);
      undo_read16(tex,pos);
      undo_read16(sector,pos);
      undo_read32(dir,pos);
      undo_read32(z,pos);
      undo_read32(y,pos);
      undo_read32(x,pos);
      undo_read32(sprite,pos);

      redo_write32(sprite);

      map->sprite_count++;
      map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count,"Map sprites grow");
      map->sprites[map->sprite_count-1] = map->sprites[sprite];
      map->sprites[sprite].x = x;
      map->sprites[sprite].y = y;
      map->sprites[sprite].z = z;
      map->sprites[sprite].dir = dir;
      map->sprites[sprite].sector = sector;
      map->sprites[sprite].tex = tex;
      map->sprites[sprite].flags = flags;
      map->sprites[sprite].x_units = x_units;
      map->sprites[sprite].y_units = y_units;
      //redo_write32(map->sprites[sprite].x);
      //redo_write32(map->sprites[sprite].y);
      //redo_write32(map->sprites[sprite].z);
      //redo_write32(map->sprites[sprite].dir);
      //redo_write16(map->sprites[sprite].sector);
      //redo_write16(map->sprites[sprite].tex);
      //redo_write32(map->sprites[sprite].flags);
      //redo_write8(map->sprites[sprite].x_units);
      //redo_write8(map->sprites[sprite].y_units);

   }
}

static void redo_sprite_del(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sprite;

      redo_read32(sprite,pos);

      undo_write32(sprite);
      undo_write32(map->sprites[sprite].x);
      undo_write32(map->sprites[sprite].y);
      undo_write32(map->sprites[sprite].z);
      undo_write32(map->sprites[sprite].dir);
      undo_write16(map->sprites[sprite].sector);
      undo_write16(map->sprites[sprite].tex);
      undo_write32(map->sprites[sprite].flags);
      undo_write8(map->sprites[sprite].x_units);
      undo_write8(map->sprites[sprite].y_units);

      map->sprites[sprite] = map->sprites[map->sprite_count-1];
      map->sprite_count--;
      map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count,"Map sprites grow");
   }
}

void undo_track_wall_move(uint32_t wall)
{
   undo_begin(ED_WALL_MOVE);
   undo_write32(wall);
   undo_write32(map->walls[wall].x);
   undo_write32(map->walls[wall].y);
   undo_end();
}

static void undo_wall_move(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint32_t x;
      uint32_t y;
      undo_read32(y,pos);
      undo_read32(x,pos);
      undo_read32(wall,pos);

      redo_write32(wall);
      redo_write32(map->walls[wall].x);
      redo_write32(map->walls[wall].y);

      RvR_port_wall_move(map, wall, x, y);
   }
}

static void redo_wall_move(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t wall;
      uint32_t x;
      uint32_t y;
      redo_read32(y,pos);
      redo_read32(x,pos);
      redo_read32(wall,pos);

      undo_write32(wall);
      undo_write32(map->walls[wall].x);
      undo_write32(map->walls[wall].y);

      RvR_port_wall_move(map, wall, x, y);
   }
}

void undo_track_sector_add(uint32_t sector)
{
   undo_begin(ED_SECTOR_ADD);
   undo_write32(sector);
   undo_end();
}

static void undo_sector_add(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      undo_read32(sector,pos);

      if(sector!=map->sector_count-1)
         RvR_log_line("undo_sector_add","Sector mismatch, expected sector '%"PRIu32"', got '%"PRIu32"'\n",map->sector_count-1,sector);

      redo_write32(sector);
      for(int i = 0;i<map->sectors[sector].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector].wall_first+i;
         redo_write32(map->walls[wall].x);
         redo_write32(map->walls[wall].y);
         redo_write32(map->walls[wall].flags);
         redo_write32(map->walls[wall].p2);
         redo_write32(map->walls[wall].portal);
         redo_write32(map->walls[wall].portal_wall);
         redo_write16(map->walls[wall].tex_lower);
         redo_write16(map->walls[wall].tex_upper);
         redo_write16(map->walls[wall].tex_mid);
         redo_write8(map->walls[wall].shade_offset);
         redo_write16(map->walls[wall].x_off);
         redo_write16(map->walls[wall].y_off);
         redo_write8(map->walls[wall].x_units);
         redo_write8(map->walls[wall].y_units);
      }
      redo_write32(map->sectors[sector].wall_count);
      redo_write32(map->sectors[sector].wall_first);
      redo_write32(map->sectors[sector].flags);
      redo_write32(map->sectors[sector].floor);
      redo_write32(map->sectors[sector].ceiling);
      redo_write16(map->sectors[sector].floor_tex);
      redo_write16(map->sectors[sector].ceiling_tex);
      redo_write8(map->sectors[sector].shade_floor);
      redo_write8(map->sectors[sector].shade_ceiling);
      redo_write16(map->sectors[sector].slope_floor);
      redo_write16(map->sectors[sector].slope_ceiling);
      redo_write16(map->sectors[sector].x_off);
      redo_write16(map->sectors[sector].y_off);
      redo_write8(map->sectors[sector].x_units);
      redo_write8(map->sectors[sector].y_units);
      
      map->wall_count-=map->sectors[map->sector_count-1].wall_count;
      map->sector_count--;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
   }
}

static void redo_sector_add(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector;

      redo_read8(sector.y_units,pos);
      redo_read8(sector.x_units,pos);
      redo_read16(sector.y_off,pos);
      redo_read16(sector.x_off,pos);
      redo_read16(sector.slope_ceiling,pos);
      redo_read16(sector.slope_floor,pos);
      redo_read8(sector.shade_ceiling,pos);
      redo_read8(sector.shade_floor,pos);
      redo_read16(sector.ceiling_tex,pos);
      redo_read16(sector.floor_tex,pos);
      redo_read32(sector.ceiling,pos);
      redo_read32(sector.floor,pos);
      redo_read32(sector.flags,pos);
      redo_read32(sector.wall_first,pos);
      redo_read32(sector.wall_count,pos);

      map->sector_count++;
      map->wall_count+=sector.wall_count;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
      map->sectors[map->sector_count-1] = sector;

      for(int i = 0;i<sector.wall_count;i++)
      {
         RvR_port_wall wall;
         redo_read8(wall.y_units,pos);
         redo_read8(wall.x_units,pos);
         redo_read16(wall.y_off,pos);
         redo_read16(wall.x_off,pos);
         redo_read8(wall.shade_offset,pos);
         redo_read16(wall.tex_mid,pos);
         redo_read16(wall.tex_upper,pos);
         redo_read16(wall.tex_lower,pos);
         redo_read32(wall.portal_wall,pos);
         redo_read32(wall.portal,pos);
         redo_read32(wall.p2,pos);
         redo_read32(wall.flags,pos);
         redo_read32(wall.y,pos);
         redo_read32(wall.x,pos);
         map->walls[map->wall_count-1-i] = wall;
      }

      redo_read32(sector_id,pos);
      if(sector_id!=map->sector_count-1)
         RvR_log_line("redo_sector_add","Sector mismatch, expected sector '%"PRIu32"', got '%"PRIu32"'\n",map->sector_count-1,sector);

      undo_write32(sector_id);
   }
}

void undo_track_sector_add_inner(uint32_t sector)
{
   undo_begin(ED_SECTOR_ADD_INNER);
   for(int i = 0;i<map->sectors[sector].wall_count;i++)
   {
      uint32_t wall = map->sectors[sector].wall_first+i;
      undo_write32(map->walls[wall].x);
      undo_write32(map->walls[wall].y);
      undo_write32(map->walls[wall].flags);
      undo_write32(map->walls[wall].p2);
      undo_write32(map->walls[wall].portal);
      undo_write32(map->walls[wall].portal_wall);
      undo_write16(map->walls[wall].tex_lower);
      undo_write16(map->walls[wall].tex_upper);
      undo_write16(map->walls[wall].tex_mid);
      undo_write8(map->walls[wall].shade_offset);
      undo_write16(map->walls[wall].x_off);
      undo_write16(map->walls[wall].y_off);
      undo_write8(map->walls[wall].x_units);
      undo_write8(map->walls[wall].y_units);
   }
   undo_write32(map->sectors[sector].wall_count);
   undo_write32(map->sectors[sector].wall_first);
   undo_write32(map->sectors[sector].flags);
   undo_write32(map->sectors[sector].floor);
   undo_write32(map->sectors[sector].ceiling);
   undo_write16(map->sectors[sector].floor_tex);
   undo_write16(map->sectors[sector].ceiling_tex);
   undo_write8(map->sectors[sector].shade_floor);
   undo_write8(map->sectors[sector].shade_ceiling);
   undo_write16(map->sectors[sector].slope_floor);
   undo_write16(map->sectors[sector].slope_ceiling);
   undo_write16(map->sectors[sector].x_off);
   undo_write16(map->sectors[sector].y_off);
   undo_write8(map->sectors[sector].x_units);
   undo_write8(map->sectors[sector].y_units);
   undo_write32(sector);
   undo_end();
}

static void undo_sector_add_inner(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector;

      undo_read32(sector_id,pos);
      undo_read8(sector.y_units,pos);
      undo_read8(sector.x_units,pos);
      undo_read16(sector.y_off,pos);
      undo_read16(sector.x_off,pos);
      undo_read16(sector.slope_ceiling,pos);
      undo_read16(sector.slope_floor,pos);
      undo_read8(sector.shade_ceiling,pos);
      undo_read8(sector.shade_floor,pos);
      undo_read16(sector.ceiling_tex,pos);
      undo_read16(sector.floor_tex,pos);
      undo_read32(sector.ceiling,pos);
      undo_read32(sector.floor,pos);
      undo_read32(sector.flags,pos);
      undo_read32(sector.wall_first,pos);
      undo_read32(sector.wall_count,pos);

      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         redo_write32(map->walls[wall].x);
         redo_write32(map->walls[wall].y);
         redo_write32(map->walls[wall].flags);
         redo_write32(map->walls[wall].p2);
         redo_write32(map->walls[wall].portal);
         redo_write32(map->walls[wall].portal_wall);
         redo_write16(map->walls[wall].tex_lower);
         redo_write16(map->walls[wall].tex_upper);
         redo_write16(map->walls[wall].tex_mid);
         redo_write8(map->walls[wall].shade_offset);
         redo_write16(map->walls[wall].x_off);
         redo_write16(map->walls[wall].y_off);
         redo_write8(map->walls[wall].x_units);
         redo_write8(map->walls[wall].y_units);
      }
      redo_write32(map->sectors[sector_id].wall_count);
      redo_write32(map->sectors[sector_id].wall_first);
      redo_write32(map->sectors[sector_id].flags);
      redo_write32(map->sectors[sector_id].floor);
      redo_write32(map->sectors[sector_id].ceiling);
      redo_write16(map->sectors[sector_id].floor_tex);
      redo_write16(map->sectors[sector_id].ceiling_tex);
      redo_write8(map->sectors[sector_id].shade_floor);
      redo_write8(map->sectors[sector_id].shade_ceiling);
      redo_write16(map->sectors[sector_id].slope_floor);
      redo_write16(map->sectors[sector_id].slope_ceiling);
      redo_write16(map->sectors[sector_id].x_off);
      redo_write16(map->sectors[sector_id].y_off);
      redo_write8(map->sectors[sector_id].x_units);
      redo_write8(map->sectors[sector_id].y_units);
      redo_write32(sector_id);

      int diff = map->sectors[sector_id].wall_count-sector.wall_count;

      uint32_t remove = map->sectors[sector_id].wall_first+sector.wall_count;
      uint32_t count = diff;

      //Move existing walls to left
      for(int w = remove;w<map->wall_count-count;w++)
         map->walls[w] = map->walls[w + count];
      
      //Fix indices
      for(int w = 0; w<map->wall_count; w++)
      {
         RvR_port_wall *wall = map->walls+w;
         if(wall->p2>=remove)
            wall->p2-=count;
         if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=remove)
            wall->portal_wall-=count;
      }

      //Update sectors first walls
      for(int s = 0;s<map->sector_count;s++)
      {
         RvR_port_sector *sct = map->sectors+s;
         if(sct->wall_first>=remove)
            sct->wall_first-=count;
      }

      map->wall_count-=count;
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
      map->sectors[sector_id] = sector;
      for(int i = 0;i<sector.wall_count;i++)
      {
         RvR_port_wall wall;
         undo_read8(wall.y_units,pos);
         undo_read8(wall.x_units,pos);
         undo_read16(wall.y_off,pos);
         undo_read16(wall.x_off,pos);
         undo_read8(wall.shade_offset,pos);
         undo_read16(wall.tex_mid,pos);
         undo_read16(wall.tex_upper,pos);
         undo_read16(wall.tex_lower,pos);
         undo_read32(wall.portal_wall,pos);
         undo_read32(wall.portal,pos);
         undo_read32(wall.p2,pos);
         undo_read32(wall.flags,pos);
         undo_read32(wall.y,pos);
         undo_read32(wall.x,pos);
         map->walls[map->sectors[sector_id].wall_first+map->sectors[sector_id].wall_count-1-i] = wall;
      }
   }
}

static void redo_sector_add_inner(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector;

      redo_read32(sector_id,pos);
      redo_read8(sector.y_units,pos);
      redo_read8(sector.x_units,pos);
      redo_read16(sector.y_off,pos);
      redo_read16(sector.x_off,pos);
      redo_read16(sector.slope_ceiling,pos);
      redo_read16(sector.slope_floor,pos);
      redo_read8(sector.shade_ceiling,pos);
      redo_read8(sector.shade_floor,pos);
      redo_read16(sector.ceiling_tex,pos);
      redo_read16(sector.floor_tex,pos);
      redo_read32(sector.ceiling,pos);
      redo_read32(sector.floor,pos);
      redo_read32(sector.flags,pos);
      redo_read32(sector.wall_first,pos);
      redo_read32(sector.wall_count,pos);

      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         undo_write32(map->walls[wall].x);
         undo_write32(map->walls[wall].y);
         undo_write32(map->walls[wall].flags);
         undo_write32(map->walls[wall].p2);
         undo_write32(map->walls[wall].portal);
         undo_write32(map->walls[wall].portal_wall);
         undo_write16(map->walls[wall].tex_lower);
         undo_write16(map->walls[wall].tex_upper);
         undo_write16(map->walls[wall].tex_mid);
         undo_write8(map->walls[wall].shade_offset);
         undo_write16(map->walls[wall].x_off);
         undo_write16(map->walls[wall].y_off);
         undo_write8(map->walls[wall].x_units);
         undo_write8(map->walls[wall].y_units);
      }
      undo_write32(map->sectors[sector_id].wall_count);
      undo_write32(map->sectors[sector_id].wall_first);
      undo_write32(map->sectors[sector_id].flags);
      undo_write32(map->sectors[sector_id].floor);
      undo_write32(map->sectors[sector_id].ceiling);
      undo_write16(map->sectors[sector_id].floor_tex);
      undo_write16(map->sectors[sector_id].ceiling_tex);
      undo_write8(map->sectors[sector_id].shade_floor);
      undo_write8(map->sectors[sector_id].shade_ceiling);
      undo_write16(map->sectors[sector_id].slope_floor);
      undo_write16(map->sectors[sector_id].slope_ceiling);
      undo_write16(map->sectors[sector_id].x_off);
      undo_write16(map->sectors[sector_id].y_off);
      undo_write8(map->sectors[sector_id].x_units);
      undo_write8(map->sectors[sector_id].y_units);
      undo_write32(sector_id);

      int diff = sector.wall_count-map->sectors[sector_id].wall_count;
      uint32_t insert = map->sectors[sector_id].wall_first+map->sectors[sector_id].wall_count;
      uint32_t count = diff;
      map->wall_count+=diff;
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");

      //Move existing walls to right
      for(int w = map->wall_count - 1; w>insert; w--)
         map->walls[w] = map->walls[w - count];

      //Fix indices
      for(int i = 0; i<map->wall_count; i++)
      {
         RvR_port_wall *wall = map->walls + i;
         if(wall->p2>=insert)
            wall->p2 += count;
         if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=insert)
            wall->portal_wall += count;
      }

      //Update sectors first wall
      for(int i = 0; i<map->sector_count; i++)
      {
         RvR_port_sector *sect = map->sectors + i;
         if(sect->wall_first>=insert)
            sect->wall_first += count;
      }

      map->sectors[sector_id] = sector;
      for(int i = 0;i<sector.wall_count;i++)
      {
         RvR_port_wall wall;
         redo_read8(wall.y_units,pos);
         redo_read8(wall.x_units,pos);
         redo_read16(wall.y_off,pos);
         redo_read16(wall.x_off,pos);
         redo_read8(wall.shade_offset,pos);
         redo_read16(wall.tex_mid,pos);
         redo_read16(wall.tex_upper,pos);
         redo_read16(wall.tex_lower,pos);
         redo_read32(wall.portal_wall,pos);
         redo_read32(wall.portal,pos);
         redo_read32(wall.p2,pos);
         redo_read32(wall.flags,pos);
         redo_read32(wall.y,pos);
         redo_read32(wall.x,pos);
         map->walls[map->sectors[sector_id].wall_first+map->sectors[sector_id].wall_count-1-i] = wall;
      }
   }
}

void undo_track_sector_add_overlap(uint32_t sector)
{
   undo_begin(ED_SECTOR_ADD_OVERLAP);
   undo_write32(sector);
   undo_end();
}

static void undo_sector_add_overlap(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector;
      undo_read32(sector,pos);

      if(sector!=map->sector_count-1)
         RvR_log_line("undo_sector_add_overlap","Sector mismatch, expected sector '%"PRIu32"', got '%"PRIu32"'\n",map->sector_count-1,sector);

      redo_write32(sector);
      for(int i = 0;i<map->sectors[sector].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector].wall_first+i;
         redo_write32(map->walls[wall].x);
         redo_write32(map->walls[wall].y);
         redo_write32(map->walls[wall].flags);
         redo_write32(map->walls[wall].p2);
         redo_write32(map->walls[wall].portal);
         redo_write32(map->walls[wall].portal_wall);
         redo_write16(map->walls[wall].tex_lower);
         redo_write16(map->walls[wall].tex_upper);
         redo_write16(map->walls[wall].tex_mid);
         redo_write8(map->walls[wall].shade_offset);
         redo_write16(map->walls[wall].x_off);
         redo_write16(map->walls[wall].y_off);
         redo_write8(map->walls[wall].x_units);
         redo_write8(map->walls[wall].y_units);
      }
      redo_write32(map->sectors[sector].wall_count);
      redo_write32(map->sectors[sector].wall_first);
      redo_write32(map->sectors[sector].flags);
      redo_write32(map->sectors[sector].floor);
      redo_write32(map->sectors[sector].ceiling);
      redo_write16(map->sectors[sector].floor_tex);
      redo_write16(map->sectors[sector].ceiling_tex);
      redo_write8(map->sectors[sector].shade_floor);
      redo_write8(map->sectors[sector].shade_ceiling);
      redo_write16(map->sectors[sector].slope_floor);
      redo_write16(map->sectors[sector].slope_ceiling);
      redo_write16(map->sectors[sector].x_off);
      redo_write16(map->sectors[sector].y_off);
      redo_write8(map->sectors[sector].x_units);
      redo_write8(map->sectors[sector].y_units);

      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID&&
            map->walls[map->walls[wall].portal_wall].portal_wall==wall)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = RVR_PORT_WALL_INVALID;
            map->walls[map->walls[wall].portal_wall].portal = RVR_PORT_SECTOR_INVALID;
         }
      }
      
      map->wall_count-=map->sectors[map->sector_count-1].wall_count;
      map->sector_count--;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
   }
}

static void redo_sector_add_overlap(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector;

      redo_read8(sector.y_units,pos);
      redo_read8(sector.x_units,pos);
      redo_read16(sector.y_off,pos);
      redo_read16(sector.x_off,pos);
      redo_read16(sector.slope_ceiling,pos);
      redo_read16(sector.slope_floor,pos);
      redo_read8(sector.shade_ceiling,pos);
      redo_read8(sector.shade_floor,pos);
      redo_read16(sector.ceiling_tex,pos);
      redo_read16(sector.floor_tex,pos);
      redo_read32(sector.ceiling,pos);
      redo_read32(sector.floor,pos);
      redo_read32(sector.flags,pos);
      redo_read32(sector.wall_first,pos);
      redo_read32(sector.wall_count,pos);

      map->sector_count++;
      map->wall_count+=sector.wall_count;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
      map->sectors[map->sector_count-1] = sector;

      for(int i = 0;i<sector.wall_count;i++)
      {
         RvR_port_wall wall;
         redo_read8(wall.y_units,pos);
         redo_read8(wall.x_units,pos);
         redo_read16(wall.y_off,pos);
         redo_read16(wall.x_off,pos);
         redo_read8(wall.shade_offset,pos);
         redo_read16(wall.tex_mid,pos);
         redo_read16(wall.tex_upper,pos);
         redo_read16(wall.tex_lower,pos);
         redo_read32(wall.portal_wall,pos);
         redo_read32(wall.portal,pos);
         redo_read32(wall.p2,pos);
         redo_read32(wall.flags,pos);
         redo_read32(wall.y,pos);
         redo_read32(wall.x,pos);
         map->walls[map->wall_count-1-i] = wall;
      }

      redo_read32(sector_id,pos);
      if(sector_id!=map->sector_count-1)
         RvR_log_line("redo_sector_add_overlap","Sector mismatch, expected sector '%"PRIu32"', got '%"PRIu32"'\n",map->sector_count-1,sector);

      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = wall;
            map->walls[map->walls[wall].portal_wall].portal = map->sector_count-1;
         }
      }

      undo_write32(sector_id);
   }
}

void undo_track_sector_split(uint32_t sector)
{
   undo_begin(ED_SECTOR_SPLIT);
   for(int i = 0;i<map->sectors[sector].wall_count;i++)
   {
      uint32_t wall = map->sectors[sector].wall_first+i;
      undo_write32(map->walls[wall].x);
      undo_write32(map->walls[wall].y);
      undo_write32(map->walls[wall].flags);
      undo_write32(map->walls[wall].p2);
      undo_write32(map->walls[wall].portal);
      undo_write32(map->walls[wall].portal_wall);
      undo_write16(map->walls[wall].tex_lower);
      undo_write16(map->walls[wall].tex_upper);
      undo_write16(map->walls[wall].tex_mid);
      undo_write8(map->walls[wall].shade_offset);
      undo_write16(map->walls[wall].x_off);
      undo_write16(map->walls[wall].y_off);
      undo_write8(map->walls[wall].x_units);
      undo_write8(map->walls[wall].y_units);
   }
   undo_write32(map->sectors[sector].wall_count);
   undo_write32(map->sectors[sector].wall_first);
   undo_write32(map->sectors[sector].flags);
   undo_write32(map->sectors[sector].floor);
   undo_write32(map->sectors[sector].ceiling);
   undo_write16(map->sectors[sector].floor_tex);
   undo_write16(map->sectors[sector].ceiling_tex);
   undo_write8(map->sectors[sector].shade_floor);
   undo_write8(map->sectors[sector].shade_ceiling);
   undo_write16(map->sectors[sector].slope_floor);
   undo_write16(map->sectors[sector].slope_ceiling);
   undo_write16(map->sectors[sector].x_off);
   undo_write16(map->sectors[sector].y_off);
   undo_write8(map->sectors[sector].x_units);
   undo_write8(map->sectors[sector].y_units);
   undo_write32(sector);
   undo_end();
}

static void undo_sector_split(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector;

      undo_read32(sector_id,pos);
      undo_read8(sector.y_units,pos);
      undo_read8(sector.x_units,pos);
      undo_read16(sector.y_off,pos);
      undo_read16(sector.x_off,pos);
      undo_read16(sector.slope_ceiling,pos);
      undo_read16(sector.slope_floor,pos);
      undo_read8(sector.shade_ceiling,pos);
      undo_read8(sector.shade_floor,pos);
      undo_read16(sector.ceiling_tex,pos);
      undo_read16(sector.floor_tex,pos);
      undo_read32(sector.ceiling,pos);
      undo_read32(sector.floor,pos);
      undo_read32(sector.flags,pos);
      undo_read32(sector.wall_first,pos);
      undo_read32(sector.wall_count,pos);

      for(int i = 0;i<map->sectors[map->sector_count-2].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-2].wall_first+i;
         redo_write32(map->walls[wall].x);
         redo_write32(map->walls[wall].y);
         redo_write32(map->walls[wall].flags);
         redo_write32(map->walls[wall].p2);
         redo_write32(map->walls[wall].portal);
         redo_write32(map->walls[wall].portal_wall);
         redo_write16(map->walls[wall].tex_lower);
         redo_write16(map->walls[wall].tex_upper);
         redo_write16(map->walls[wall].tex_mid);
         redo_write8(map->walls[wall].shade_offset);
         redo_write16(map->walls[wall].x_off);
         redo_write16(map->walls[wall].y_off);
         redo_write8(map->walls[wall].x_units);
         redo_write8(map->walls[wall].y_units);
      }
      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         redo_write32(map->walls[wall].x);
         redo_write32(map->walls[wall].y);
         redo_write32(map->walls[wall].flags);
         redo_write32(map->walls[wall].p2);
         redo_write32(map->walls[wall].portal);
         redo_write32(map->walls[wall].portal_wall);
         redo_write16(map->walls[wall].tex_lower);
         redo_write16(map->walls[wall].tex_upper);
         redo_write16(map->walls[wall].tex_mid);
         redo_write8(map->walls[wall].shade_offset);
         redo_write16(map->walls[wall].x_off);
         redo_write16(map->walls[wall].y_off);
         redo_write8(map->walls[wall].x_units);
         redo_write8(map->walls[wall].y_units);
      }
      redo_write32(map->sectors[map->sector_count-2].wall_count);
      redo_write32(map->sectors[map->sector_count-2].wall_first);
      redo_write32(map->sectors[map->sector_count-2].flags);
      redo_write32(map->sectors[map->sector_count-2].floor);
      redo_write32(map->sectors[map->sector_count-2].ceiling);
      redo_write16(map->sectors[map->sector_count-2].floor_tex);
      redo_write16(map->sectors[map->sector_count-2].ceiling_tex);
      redo_write8(map->sectors[map->sector_count-2].shade_floor);
      redo_write8(map->sectors[map->sector_count-2].shade_ceiling);
      redo_write16(map->sectors[map->sector_count-2].slope_floor);
      redo_write16(map->sectors[map->sector_count-2].slope_ceiling);
      redo_write16(map->sectors[map->sector_count-2].x_off);
      redo_write16(map->sectors[map->sector_count-2].y_off);
      redo_write8(map->sectors[map->sector_count-2].x_units);
      redo_write8(map->sectors[map->sector_count-2].y_units);
      redo_write32(map->sectors[map->sector_count-1].wall_count);
      redo_write32(map->sectors[map->sector_count-1].wall_first);
      redo_write32(map->sectors[map->sector_count-1].flags);
      redo_write32(map->sectors[map->sector_count-1].floor);
      redo_write32(map->sectors[map->sector_count-1].ceiling);
      redo_write16(map->sectors[map->sector_count-1].floor_tex);
      redo_write16(map->sectors[map->sector_count-1].ceiling_tex);
      redo_write8(map->sectors[map->sector_count-1].shade_floor);
      redo_write8(map->sectors[map->sector_count-1].shade_ceiling);
      redo_write16(map->sectors[map->sector_count-1].slope_floor);
      redo_write16(map->sectors[map->sector_count-1].slope_ceiling);
      redo_write16(map->sectors[map->sector_count-1].x_off);
      redo_write16(map->sectors[map->sector_count-1].y_off);
      redo_write8(map->sectors[map->sector_count-1].x_units);
      redo_write8(map->sectors[map->sector_count-1].y_units);
      redo_write32(sector_id);

      //Remove old sectors
      //-------------------------------------
      for(int i = 0;i<map->sectors[map->sector_count-2].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-2].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID&&
            map->walls[map->walls[wall].portal_wall].portal_wall==wall)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = RVR_PORT_WALL_INVALID;
            map->walls[map->walls[wall].portal_wall].portal = RVR_PORT_SECTOR_INVALID;
         }
      }
      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID&&
            map->walls[map->walls[wall].portal_wall].portal_wall==wall)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = RVR_PORT_WALL_INVALID;
            map->walls[map->walls[wall].portal_wall].portal = RVR_PORT_SECTOR_INVALID;
         }
      }
      map->wall_count-=map->sectors[map->sector_count-2].wall_count+map->sectors[map->sector_count-1].wall_count;
      map->sector_count-=2;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
      //-------------------------------------

      //Move sectors to right
      map->sector_count+=1;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      for(int s = map->sector_count-1;s>sector_id;s--)
         map->sectors[s] = map->sectors[s-1];

      //Fix portals
      for(int w = 0;w<map->wall_count;w++)
      {
         RvR_port_wall *wall = map->walls+w;
         if(wall->portal!=RVR_PORT_SECTOR_INVALID&&wall->portal>=sector_id)
            wall->portal++;
      }

      uint32_t insert = sector.wall_first;
      uint32_t count = sector.wall_count;
      map->wall_count+=count;
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");

      //Move existing walls to right
      for(int w = map->wall_count - 1; w>insert+count-1; w--)
         map->walls[w] = map->walls[w - count];

      //Fix indices
      for(int i = 0; i<map->wall_count; i++)
      {
         RvR_port_wall *wall = map->walls + i;
         if(wall->p2>=insert)
            wall->p2 += count;
         if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=insert)
            wall->portal_wall += count;
      }

      //Update sectors first wall
      for(int i = 0; i<map->sector_count; i++)
      {
         RvR_port_sector *sect = map->sectors + i;
         if(sect->wall_first>=insert)
            sect->wall_first += count;
      }

      map->sectors[sector_id] = sector;
      for(int i = 0;i<sector.wall_count;i++)
      {
         RvR_port_wall wall;
         undo_read8(wall.y_units,pos);
         undo_read8(wall.x_units,pos);
         undo_read16(wall.y_off,pos);
         undo_read16(wall.x_off,pos);
         undo_read8(wall.shade_offset,pos);
         undo_read16(wall.tex_mid,pos);
         undo_read16(wall.tex_upper,pos);
         undo_read16(wall.tex_lower,pos);
         undo_read32(wall.portal_wall,pos);
         undo_read32(wall.portal,pos);
         undo_read32(wall.p2,pos);
         undo_read32(wall.flags,pos);
         undo_read32(wall.y,pos);
         undo_read32(wall.x,pos);
         map->walls[map->sectors[sector_id].wall_first+map->sectors[sector_id].wall_count-1-i] = wall;
      }
      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = wall;
            map->walls[map->walls[wall].portal_wall].portal = sector_id;
         }
      }
   }
}

static void redo_sector_split(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector0;
      RvR_port_sector sector1;

      redo_read32(sector_id,pos);
      redo_read8(sector0.y_units,pos);
      redo_read8(sector0.x_units,pos);
      redo_read16(sector0.y_off,pos);
      redo_read16(sector0.x_off,pos);
      redo_read16(sector0.slope_ceiling,pos);
      redo_read16(sector0.slope_floor,pos);
      redo_read8(sector0.shade_ceiling,pos);
      redo_read8(sector0.shade_floor,pos);
      redo_read16(sector0.ceiling_tex,pos);
      redo_read16(sector0.floor_tex,pos);
      redo_read32(sector0.ceiling,pos);
      redo_read32(sector0.floor,pos);
      redo_read32(sector0.flags,pos);
      redo_read32(sector0.wall_first,pos);
      redo_read32(sector0.wall_count,pos);
      redo_read8(sector1.y_units,pos);
      redo_read8(sector1.x_units,pos);
      redo_read16(sector1.y_off,pos);
      redo_read16(sector1.x_off,pos);
      redo_read16(sector1.slope_ceiling,pos);
      redo_read16(sector1.slope_floor,pos);
      redo_read8(sector1.shade_ceiling,pos);
      redo_read8(sector1.shade_floor,pos);
      redo_read16(sector1.ceiling_tex,pos);
      redo_read16(sector1.floor_tex,pos);
      redo_read32(sector1.ceiling,pos);
      redo_read32(sector1.floor,pos);
      redo_read32(sector1.flags,pos);
      redo_read32(sector1.wall_first,pos);
      redo_read32(sector1.wall_count,pos);

      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         undo_write32(map->walls[wall].x);
         undo_write32(map->walls[wall].y);
         undo_write32(map->walls[wall].flags);
         undo_write32(map->walls[wall].p2);
         undo_write32(map->walls[wall].portal);
         undo_write32(map->walls[wall].portal_wall);
         undo_write16(map->walls[wall].tex_lower);
         undo_write16(map->walls[wall].tex_upper);
         undo_write16(map->walls[wall].tex_mid);
         undo_write8(map->walls[wall].shade_offset);
         undo_write16(map->walls[wall].x_off);
         undo_write16(map->walls[wall].y_off);
         undo_write8(map->walls[wall].x_units);
         undo_write8(map->walls[wall].y_units);
      }
      undo_write32(map->sectors[sector_id].wall_count);
      undo_write32(map->sectors[sector_id].wall_first);
      undo_write32(map->sectors[sector_id].flags);
      undo_write32(map->sectors[sector_id].floor);
      undo_write32(map->sectors[sector_id].ceiling);
      undo_write16(map->sectors[sector_id].floor_tex);
      undo_write16(map->sectors[sector_id].ceiling_tex);
      undo_write8(map->sectors[sector_id].shade_floor);
      undo_write8(map->sectors[sector_id].shade_ceiling);
      undo_write16(map->sectors[sector_id].slope_floor);
      undo_write16(map->sectors[sector_id].slope_ceiling);
      undo_write16(map->sectors[sector_id].x_off);
      undo_write16(map->sectors[sector_id].y_off);
      undo_write8(map->sectors[sector_id].x_units);
      undo_write8(map->sectors[sector_id].y_units);
      undo_write32(sector_id);

      //Remove old sector
      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID&&
            map->walls[map->walls[wall].portal_wall].portal_wall==wall)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = RVR_PORT_WALL_INVALID;
            map->walls[map->walls[wall].portal_wall].portal = RVR_PORT_SECTOR_INVALID;
         }
      }

      //Move walls after deleted sector to left
      uint32_t count = map->sectors[sector_id].wall_count;
      uint32_t remove = map->sectors[sector_id].wall_first;
      for(int w = remove;w<map->wall_count-count;w++)
         map->walls[w] = map->walls[w+count];

      //Update wall references
      for(int w = 0;w<map->wall_count;w++)
      {
         RvR_port_wall *wall = map->walls+w;
         if(wall->p2>=remove)
            wall->p2-=count;
         if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=remove)
            wall->portal_wall-=count;
      }

      //Update sector references
      for(int s = 0;s<map->sector_count;s++)
      {
         if(map->sectors[s].wall_first>=remove)
            map->sectors[s].wall_first-=count;
      }

      //Move sectors after deleted sector to left
      for(int s = sector_id;s<map->sector_count-1;s++)
         map->sectors[s] = map->sectors[s+1];

      //Update portals
      for(int w = 0;w<map->wall_count;w++)
      {
         if(map->walls[w].portal!=RVR_PORT_SECTOR_INVALID&&map->walls[w].portal>=sector_id)
            map->walls[w].portal--;
      }
      map->wall_count-=count;
      map->sector_count-=1;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");

      map->sector_count+=2;
      map->wall_count+=sector0.wall_count+sector1.wall_count;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
      map->sectors[map->sector_count-1] = sector0;
      map->sectors[map->sector_count-2] = sector1;

      for(int i = 0;i<sector0.wall_count;i++)
      {
         RvR_port_wall wall;
         redo_read8(wall.y_units,pos);
         redo_read8(wall.x_units,pos);
         redo_read16(wall.y_off,pos);
         redo_read16(wall.x_off,pos);
         redo_read8(wall.shade_offset,pos);
         redo_read16(wall.tex_mid,pos);
         redo_read16(wall.tex_upper,pos);
         redo_read16(wall.tex_lower,pos);
         redo_read32(wall.portal_wall,pos);
         redo_read32(wall.portal,pos);
         redo_read32(wall.p2,pos);
         redo_read32(wall.flags,pos);
         redo_read32(wall.y,pos);
         redo_read32(wall.x,pos);
         map->walls[map->sectors[map->sector_count-1].wall_first+map->sectors[map->sector_count-1].wall_count-1-i] = wall;
      }
      for(int i = 0;i<sector1.wall_count;i++)
      {
         RvR_port_wall wall;
         redo_read8(wall.y_units,pos);
         redo_read8(wall.x_units,pos);
         redo_read16(wall.y_off,pos);
         redo_read16(wall.x_off,pos);
         redo_read8(wall.shade_offset,pos);
         redo_read16(wall.tex_mid,pos);
         redo_read16(wall.tex_upper,pos);
         redo_read16(wall.tex_lower,pos);
         redo_read32(wall.portal_wall,pos);
         redo_read32(wall.portal,pos);
         redo_read32(wall.p2,pos);
         redo_read32(wall.flags,pos);
         redo_read32(wall.y,pos);
         redo_read32(wall.x,pos);
         map->walls[map->sectors[map->sector_count-2].wall_first+map->sectors[map->sector_count-2].wall_count-1-i] = wall;
      }

      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = wall;
            map->walls[map->walls[wall].portal_wall].portal = map->sector_count-1;
         }
      }
      for(int i = 0;i<map->sectors[map->sector_count-2].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-2].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = wall;
            map->walls[map->walls[wall].portal_wall].portal = map->sector_count-2;
         }
      }
   }
}

void undo_track_sector_connect(uint32_t sector)
{
   undo_begin(ED_SECTOR_CONNECT);
   for(int i = 0;i<map->sectors[sector].wall_count;i++)
   {
      uint32_t wall = map->sectors[sector].wall_first+i;
      undo_write32(map->walls[wall].x);
      undo_write32(map->walls[wall].y);
      undo_write32(map->walls[wall].flags);
      undo_write32(map->walls[wall].p2);
      undo_write32(map->walls[wall].portal);
      undo_write32(map->walls[wall].portal_wall);
      undo_write16(map->walls[wall].tex_lower);
      undo_write16(map->walls[wall].tex_upper);
      undo_write16(map->walls[wall].tex_mid);
      undo_write8(map->walls[wall].shade_offset);
      undo_write16(map->walls[wall].x_off);
      undo_write16(map->walls[wall].y_off);
      undo_write8(map->walls[wall].x_units);
      undo_write8(map->walls[wall].y_units);
   }
   undo_write32(map->sectors[sector].wall_count);
   undo_write32(map->sectors[sector].wall_first);
   undo_write32(map->sectors[sector].flags);
   undo_write32(map->sectors[sector].floor);
   undo_write32(map->sectors[sector].ceiling);
   undo_write16(map->sectors[sector].floor_tex);
   undo_write16(map->sectors[sector].ceiling_tex);
   undo_write8(map->sectors[sector].shade_floor);
   undo_write8(map->sectors[sector].shade_ceiling);
   undo_write16(map->sectors[sector].slope_floor);
   undo_write16(map->sectors[sector].slope_ceiling);
   undo_write16(map->sectors[sector].x_off);
   undo_write16(map->sectors[sector].y_off);
   undo_write8(map->sectors[sector].x_units);
   undo_write8(map->sectors[sector].y_units);
   undo_write32(sector);
   undo_end();
}

static void undo_sector_connect(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector;

      undo_read32(sector_id,pos);
      undo_read8(sector.y_units,pos);
      undo_read8(sector.x_units,pos);
      undo_read16(sector.y_off,pos);
      undo_read16(sector.x_off,pos);
      undo_read16(sector.slope_ceiling,pos);
      undo_read16(sector.slope_floor,pos);
      undo_read8(sector.shade_ceiling,pos);
      undo_read8(sector.shade_floor,pos);
      undo_read16(sector.ceiling_tex,pos);
      undo_read16(sector.floor_tex,pos);
      undo_read32(sector.ceiling,pos);
      undo_read32(sector.floor,pos);
      undo_read32(sector.flags,pos);
      undo_read32(sector.wall_first,pos);
      undo_read32(sector.wall_count,pos);

      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         redo_write32(map->walls[wall].x);
         redo_write32(map->walls[wall].y);
         redo_write32(map->walls[wall].flags);
         redo_write32(map->walls[wall].p2);
         redo_write32(map->walls[wall].portal);
         redo_write32(map->walls[wall].portal_wall);
         redo_write16(map->walls[wall].tex_lower);
         redo_write16(map->walls[wall].tex_upper);
         redo_write16(map->walls[wall].tex_mid);
         redo_write8(map->walls[wall].shade_offset);
         redo_write16(map->walls[wall].x_off);
         redo_write16(map->walls[wall].y_off);
         redo_write8(map->walls[wall].x_units);
         redo_write8(map->walls[wall].y_units);
      }
      redo_write32(map->sectors[map->sector_count-1].wall_count);
      redo_write32(map->sectors[map->sector_count-1].wall_first);
      redo_write32(map->sectors[map->sector_count-1].flags);
      redo_write32(map->sectors[map->sector_count-1].floor);
      redo_write32(map->sectors[map->sector_count-1].ceiling);
      redo_write16(map->sectors[map->sector_count-1].floor_tex);
      redo_write16(map->sectors[map->sector_count-1].ceiling_tex);
      redo_write8(map->sectors[map->sector_count-1].shade_floor);
      redo_write8(map->sectors[map->sector_count-1].shade_ceiling);
      redo_write16(map->sectors[map->sector_count-1].slope_floor);
      redo_write16(map->sectors[map->sector_count-1].slope_ceiling);
      redo_write16(map->sectors[map->sector_count-1].x_off);
      redo_write16(map->sectors[map->sector_count-1].y_off);
      redo_write8(map->sectors[map->sector_count-1].x_units);
      redo_write8(map->sectors[map->sector_count-1].y_units);
      redo_write32(sector_id);

      //Remove old sectors
      //-------------------------------------
      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID&&
            map->walls[map->walls[wall].portal_wall].portal_wall==wall)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = RVR_PORT_WALL_INVALID;
            map->walls[map->walls[wall].portal_wall].portal = RVR_PORT_SECTOR_INVALID;
         }
      }
      map->wall_count-=map->sectors[map->sector_count-1].wall_count;
      map->sector_count-=1;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
      //-------------------------------------

      //Move sectors to right
      map->sector_count+=1;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      for(int s = map->sector_count-1;s>sector_id;s--)
         map->sectors[s] = map->sectors[s-1];

      //Fix portals
      for(int w = 0;w<map->wall_count;w++)
      {
         RvR_port_wall *wall = map->walls+w;
         if(wall->portal!=RVR_PORT_SECTOR_INVALID&&wall->portal>=sector_id)
            wall->portal++;
      }

      uint32_t insert = sector.wall_first;
      uint32_t count = sector.wall_count;
      map->wall_count+=count;
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");

      //Move existing walls to right
      for(int w = map->wall_count - 1; w>insert+count-1; w--)
         map->walls[w] = map->walls[w - count];

      //Fix indices
      for(int i = 0; i<map->wall_count; i++)
      {
         RvR_port_wall *wall = map->walls + i;
         if(wall->p2>=insert)
            wall->p2 += count;
         if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=insert)
            wall->portal_wall += count;
      }

      //Update sectors first wall
      for(int i = 0; i<map->sector_count; i++)
      {
         RvR_port_sector *sect = map->sectors + i;
         if(sect->wall_first>=insert)
            sect->wall_first += count;
      }

      map->sectors[sector_id] = sector;
      for(int i = 0;i<sector.wall_count;i++)
      {
         RvR_port_wall wall;
         undo_read8(wall.y_units,pos);
         undo_read8(wall.x_units,pos);
         undo_read16(wall.y_off,pos);
         undo_read16(wall.x_off,pos);
         undo_read8(wall.shade_offset,pos);
         undo_read16(wall.tex_mid,pos);
         undo_read16(wall.tex_upper,pos);
         undo_read16(wall.tex_lower,pos);
         undo_read32(wall.portal_wall,pos);
         undo_read32(wall.portal,pos);
         undo_read32(wall.p2,pos);
         undo_read32(wall.flags,pos);
         undo_read32(wall.y,pos);
         undo_read32(wall.x,pos);
         map->walls[map->sectors[sector_id].wall_first+map->sectors[sector_id].wall_count-1-i] = wall;
      }
      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = wall;
            map->walls[map->walls[wall].portal_wall].portal = sector_id;
         }
      }
   }
}

static void redo_sector_connect(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t sector_id;
      RvR_port_sector sector0;

      redo_read32(sector_id,pos);
      redo_read8(sector0.y_units,pos);
      redo_read8(sector0.x_units,pos);
      redo_read16(sector0.y_off,pos);
      redo_read16(sector0.x_off,pos);
      redo_read16(sector0.slope_ceiling,pos);
      redo_read16(sector0.slope_floor,pos);
      redo_read8(sector0.shade_ceiling,pos);
      redo_read8(sector0.shade_floor,pos);
      redo_read16(sector0.ceiling_tex,pos);
      redo_read16(sector0.floor_tex,pos);
      redo_read32(sector0.ceiling,pos);
      redo_read32(sector0.floor,pos);
      redo_read32(sector0.flags,pos);
      redo_read32(sector0.wall_first,pos);
      redo_read32(sector0.wall_count,pos);

      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         undo_write32(map->walls[wall].x);
         undo_write32(map->walls[wall].y);
         undo_write32(map->walls[wall].flags);
         undo_write32(map->walls[wall].p2);
         undo_write32(map->walls[wall].portal);
         undo_write32(map->walls[wall].portal_wall);
         undo_write16(map->walls[wall].tex_lower);
         undo_write16(map->walls[wall].tex_upper);
         undo_write16(map->walls[wall].tex_mid);
         undo_write8(map->walls[wall].shade_offset);
         undo_write16(map->walls[wall].x_off);
         undo_write16(map->walls[wall].y_off);
         undo_write8(map->walls[wall].x_units);
         undo_write8(map->walls[wall].y_units);
      }
      undo_write32(map->sectors[sector_id].wall_count);
      undo_write32(map->sectors[sector_id].wall_first);
      undo_write32(map->sectors[sector_id].flags);
      undo_write32(map->sectors[sector_id].floor);
      undo_write32(map->sectors[sector_id].ceiling);
      undo_write16(map->sectors[sector_id].floor_tex);
      undo_write16(map->sectors[sector_id].ceiling_tex);
      undo_write8(map->sectors[sector_id].shade_floor);
      undo_write8(map->sectors[sector_id].shade_ceiling);
      undo_write16(map->sectors[sector_id].slope_floor);
      undo_write16(map->sectors[sector_id].slope_ceiling);
      undo_write16(map->sectors[sector_id].x_off);
      undo_write16(map->sectors[sector_id].y_off);
      undo_write8(map->sectors[sector_id].x_units);
      undo_write8(map->sectors[sector_id].y_units);
      undo_write32(sector_id);

      //Remove old sector
      for(int i = 0;i<map->sectors[sector_id].wall_count;i++)
      {
         uint32_t wall = map->sectors[sector_id].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID&&
            map->walls[map->walls[wall].portal_wall].portal_wall==wall)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = RVR_PORT_WALL_INVALID;
            map->walls[map->walls[wall].portal_wall].portal = RVR_PORT_SECTOR_INVALID;
         }
      }

      //Move walls after deleted sector to left
      uint32_t count = map->sectors[sector_id].wall_count;
      uint32_t remove = map->sectors[sector_id].wall_first;
      for(int w = remove;w<map->wall_count-count;w++)
         map->walls[w] = map->walls[w+count];

      //Update wall references
      for(int w = 0;w<map->wall_count;w++)
      {
         RvR_port_wall *wall = map->walls+w;
         if(wall->p2>=remove)
            wall->p2-=count;
         if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=remove)
            wall->portal_wall-=count;
      }

      //Update sector references
      for(int s = 0;s<map->sector_count;s++)
      {
         if(map->sectors[s].wall_first>=remove)
            map->sectors[s].wall_first-=count;
      }

      //Move sectors after deleted sector to left
      for(int s = sector_id;s<map->sector_count-1;s++)
         map->sectors[s] = map->sectors[s+1];

      //Update portals
      for(int w = 0;w<map->wall_count;w++)
      {
         if(map->walls[w].portal!=RVR_PORT_SECTOR_INVALID&&map->walls[w].portal>=sector_id)
            map->walls[w].portal--;
      }
      map->wall_count-=count;
      map->sector_count-=1;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");

      map->sector_count+=1;
      map->wall_count+=sector0.wall_count;
      map->sectors = RvR_realloc(map->sectors, sizeof(*map->sectors) * map->sector_count, "Map sectors grow");
      map->walls = RvR_realloc(map->walls, sizeof(*map->walls) * map->wall_count, "Map walls grow");
      map->sectors[map->sector_count-1] = sector0;

      for(int i = 0;i<sector0.wall_count;i++)
      {
         RvR_port_wall wall;
         redo_read8(wall.y_units,pos);
         redo_read8(wall.x_units,pos);
         redo_read16(wall.y_off,pos);
         redo_read16(wall.x_off,pos);
         redo_read8(wall.shade_offset,pos);
         redo_read16(wall.tex_mid,pos);
         redo_read16(wall.tex_upper,pos);
         redo_read16(wall.tex_lower,pos);
         redo_read32(wall.portal_wall,pos);
         redo_read32(wall.portal,pos);
         redo_read32(wall.p2,pos);
         redo_read32(wall.flags,pos);
         redo_read32(wall.y,pos);
         redo_read32(wall.x,pos);
         map->walls[map->sectors[map->sector_count-1].wall_first+map->sectors[map->sector_count-1].wall_count-1-i] = wall;
      }

      for(int i = 0;i<map->sectors[map->sector_count-1].wall_count;i++)
      {
         uint32_t wall = map->sectors[map->sector_count-1].wall_first+i;
         if(map->walls[wall].portal_wall!=RVR_PORT_WALL_INVALID)
         {
            map->walls[map->walls[wall].portal_wall].portal_wall = wall;
            map->walls[map->walls[wall].portal_wall].portal = map->sector_count-1;
         }
      }
   }
}
//-------------------------------------
