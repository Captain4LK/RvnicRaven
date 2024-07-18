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
#define undo_read16(var,pos) do { uint16_t lo16,hi16; undo_read8(hi16,pos); undo_read8(lo16,pos); var = hi16*0x100+lo16; } while(0)
#define undo_read32(var,pos) do { uint32_t lo32,hi32; undo_read16(hi32,pos); undo_read16(lo32,pos); var = hi32*0x1000f+lo32; } while(0)
#define undo_read64(var,pos) do { uint64_t lo64,hi64; undo_read32(hi64,pos); undo_read32(lo64,pos); var = hi64*0x100000000+lo64; } while(0)

#define redo_read8(var,pos) do { var = undo_buffer[pos]; pos = WRAP(pos+1); } while(0)
#define redo_read16(var,pos) do { uint16_t lo16,hi16; redo_read8(hi16,pos); redo_read8(lo16,pos); var = hi16*0x100+lo16; } while(0)
#define redo_read32(var,pos) do { uint32_t lo32,hi32; redo_read16(hi32,pos); redo_read16(lo32,pos); var = hi32*0x10000+lo32; } while(0)
#define redo_read64(var,pos) do { uint64_t lo64,hi64; redo_read32(hi64,pos); redo_read32(lo64,pos); var = hi64*0x100000000+lo64; } while(0)
//-------------------------------------

//Typedefs
typedef enum
{
   ED_WALL_SHADE = 0,
   //ED_WALL_MOVE = 0,
   //ED_SECTOR = 1,
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
//static void undo_write(uint16_t val);
static void undo_write8(uint8_t val);
static void undo_write16(uint16_t val);
static void undo_write32(uint32_t val);
static void undo_write64(uint64_t val);

static void undo_begin(Ed_action action);
static void undo_end();
static int undo_find_end(uint32_t *len);

//static void redo_write(uint16_t val);
static void redo_write8(uint8_t val);
static void redo_write16(uint16_t val);
static void redo_write32(uint32_t val);
static void redo_write64(uint64_t val);

static int redo_find_end(uint32_t *len);

static void undo_shade_wall(int pos, int endpos);
static void redo_shade_wall(int pos, int endpos);

//static void undo_move_wall(int pos, int endpos);
//static void redo_move_wall(int pos, int endpos);
//static void undo_sector(int pos, int endpos);
//static void redo_sector(int pos, int endpos);
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
   //case ED_WALL_MOVE: undo_move_wall(pos, endpos); break;
   //case ED_SECTOR: undo_sector(pos, endpos); break;
   }

   redo_write32(len);
   //redo_write16((len >> 16) & UINT16_MAX);
   //redo_write16(len & UINT16_MAX);
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
   //case ED_WALL_MOVE: redo_move_wall(pos, endpos); break;
   //case ED_SECTOR: redo_sector(pos, endpos); break;
   }

   undo_write32(len);
   //undo_write16((len >> 16) & UINT16_MAX);
   //undo_write16(len & UINT16_MAX);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

/*void undo_track_wall_move(int16_t wall, RvR_fix22 px, RvR_fix22 py)
{
   undo_begin(ED_WALL_MOVE);
   undo_write(wall);
   undo_write(px & 0xffff);
   undo_write((px >> 16) & 0xffff);
   undo_write(py & 0xffff);
   undo_write((py >> 16) & 0xffff);
   undo_end();
}

void undo_track_sector(int16_t sector)
{
   undo_begin(ED_SECTOR);
   undo_write(sector);
   undo_end();
}
*/

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

/*static void redo_write(uint16_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos - 1);
   redo_len += (redo_len<UNDO_BUFFER_SIZE - 2);
   undo_len -= (undo_len>0);
}*/

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
   //int pos = (undo_pos-1)&(UNDO_BUFFER_SIZE-1);
   if(undo_buffer[pos]==UNDO_RECORD)
   {
      //empty
      undo_pos = pos;
      undo_len -= 1;
   }
   else
   {
      //printf("write len %d\n",undo_entry_len);
      undo_write32(undo_entry_len);
      //uint16_t hi = (undo_entry_len >> 16) & UINT16_MAX;
      //uint16_t lo = undo_entry_len & UINT16_MAX;
      //undo_write16(hi);
      //undo_write16(lo);
   }
   undo_buffer[undo_pos] = JUNK_RECORD;
}

static int undo_find_end(uint32_t *len)
{
   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return -1;

   int pos = WRAP(undo_pos - 1);
   undo_read32(*len,pos);
   //*len = undo_buffer[pos]; pos = WRAP(pos - 1);
   //*len += undo_buffer[pos] << 16;

   return WRAP(pos - *len);
}

static int redo_find_end(uint32_t *len)
{
   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return -1;
   if(redo_len<=0)
      return -1;

   int pos = WRAP(undo_pos + 1);
   *len = undo_buffer[pos]; pos = WRAP(pos + 1);
   *len += undo_buffer[pos] << 16;

   return WRAP(pos + *len + 1);
}

void undo_track_wall_shade(uint32_t wall, int8_t shade)
{
   undo_begin(ED_WALL_SHADE);
   undo_write32(wall);
   undo_write8(shade);
   //undo_write(wall);
   //undo_write(px & 0xffff);
   //undo_write((px >> 16) & 0xffff);
   //undo_write(py & 0xffff);
   //undo_write((py >> 16) & 0xffff);
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

/*static void undo_move_wall(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t y = undo_buffer[pos];  pos = WRAP(pos - 1);
      y = (y << 16) + undo_buffer[pos];    pos = WRAP(pos - 1);
      uint32_t x = undo_buffer[pos];  pos = WRAP(pos - 1);
      x = (x << 16) + undo_buffer[pos];    pos = WRAP(pos - 1);
      int16_t wall = undo_buffer[pos]; pos = WRAP(pos - 1);

      RvR_port_wall *w = map->walls + wall;
      redo_write(wall);
      redo_write(w->x & 0xffff);
      redo_write((w->x >> 16) & 0xffff);
      redo_write(w->y & 0xffff);
      redo_write((w->y >> 16) & 0xffff);

      RvR_port_wall_move(map, wall, x, y);
   }
}

static void redo_move_wall(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t y = undo_buffer[pos];  pos = WRAP(pos + 1);
      y = (y << 16) + undo_buffer[pos];    pos = WRAP(pos + 1);
      uint32_t x = undo_buffer[pos];  pos = WRAP(pos + 1);
      x = (x << 16) + undo_buffer[pos];    pos = WRAP(pos + 1);
      int16_t wall = undo_buffer[pos]; pos = WRAP(pos + 1);

      RvR_port_wall *w = map->walls + wall;
      undo_write(wall);
      undo_write(w->x & 0xffff);
      undo_write((w->x >> 16) & 0xffff);
      undo_write(w->y & 0xffff);
      undo_write((w->y >> 16) & 0xffff);

      RvR_port_wall_move(map, wall, x, y);
   }
}*/
//-------------------------------------
