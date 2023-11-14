/*
RvnicRaven retro game engine

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#define UNDO_RECORD (UINT16_MAX)
#define REDO_RECORD (UINT16_MAX - 2)
#define JUNK_RECORD (UINT16_MAX - 1)
//-------------------------------------

//Typedefs
typedef enum
{
   ED_WALL_MOVE = 0,
   ED_SECTOR = 1,
}Ed_action;
//-------------------------------------

//Variables
static uint16_t *undo_buffer = NULL;
static int undo_len = 0;
static int undo_pos = 0;
static int redo_len = 0;
static uint32_t undo_entry_len = 0;
//-------------------------------------

//Function prototypes
static void undo_write(uint16_t val);
static void undo_begin(Ed_action action);
static void undo_end();
static int undo_find_end(uint32_t *len);

static void redo_write(uint16_t val);
static int redo_find_end(uint32_t *len);

static void undo_move_wall(int pos, int endpos);
static void redo_move_wall(int pos, int endpos);
static void undo_sector(int pos, int endpos);
static void redo_sector(int pos, int endpos);
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
   int pos = 0;
   uint32_t len = 0;
   int endpos = undo_find_end(&len);
   if(endpos<0)
      return;
   Ed_action action = undo_buffer[endpos];

   endpos = WRAP(endpos);
   pos = WRAP(undo_pos - 3);

   if(pos==endpos)
      return;

   //New redo entry
   redo_write(REDO_RECORD);
   redo_write(action);

   //Apply undoes
   switch(action)
   {
   case ED_WALL_MOVE: undo_move_wall(pos, endpos); break;
   case ED_SECTOR: undo_sector(pos, endpos); break;
   }

   redo_write((len >> 16) & UINT16_MAX);
   redo_write(len & UINT16_MAX);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

void redo(void)
{
   int pos = 0;
   uint32_t len = 0;
   int endpos = redo_find_end(&len);
   if(endpos<0)
      return;

   Ed_action action = undo_buffer[endpos];
   pos = WRAP(undo_pos + 3);

   if(pos==endpos)
      return;

   //New undo entry
   undo_write(UNDO_RECORD);
   undo_write(action);

   //Apply redoes
   switch(action)
   {
   case ED_WALL_MOVE: redo_move_wall(pos, endpos); break;
   case ED_SECTOR: redo_sector(pos, endpos); break;
   }

   undo_write((len >> 16) & UINT16_MAX);
   undo_write(len & UINT16_MAX);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

void undo_track_wall_move(int16_t wall, RvR_fix22 px, RvR_fix22 py)
{
   undo_begin(ED_WALL_MOVE);
   undo_write(wall);
   undo_write(px&0xffff);
   undo_write((px>>16)&0xffff);
   undo_write(py&0xffff);
   undo_write((py>>16)&0xffff);
   undo_end();
}

void undo_track_sector(int16_t sector)
{
   undo_begin(ED_SECTOR);
   undo_write(sector);
   undo_end();
}

static void undo_write(uint16_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos + 1);
   undo_len += (undo_len<UNDO_BUFFER_SIZE - 2);
   redo_len -= (redo_len>0);
   undo_entry_len++;
}

static void redo_write(uint16_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos - 1);
   redo_len += (redo_len<UNDO_BUFFER_SIZE - 2);
   undo_len -= (undo_len>0);
}

static void undo_begin(Ed_action action)
{
   redo_len = 0;
   undo_write(UNDO_RECORD);
   undo_write(action);
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
      uint16_t hi = (undo_entry_len >> 16) & UINT16_MAX;
      uint16_t lo = undo_entry_len & UINT16_MAX;
      undo_write(hi);
      undo_write(lo);
   }
   undo_buffer[undo_pos] = JUNK_RECORD;
}

static int undo_find_end(uint32_t *len)
{
   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return -1;

   int pos = WRAP(undo_pos - 1);
   *len = undo_buffer[pos]; pos = WRAP(pos - 1);
   *len += undo_buffer[pos] << 16;

   return WRAP(pos - *len - 1);
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

static void undo_move_wall(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t y = undo_buffer[pos];  pos = WRAP(pos-1);
      y = (y<<16)+undo_buffer[pos];    pos = WRAP(pos-1);
      uint32_t x = undo_buffer[pos];  pos = WRAP(pos-1);
      x = (x<<16)+undo_buffer[pos];    pos = WRAP(pos-1);
      int16_t wall = undo_buffer[pos]; pos = WRAP(pos-1);

      RvR_port_wall *w = map->walls+wall;
      redo_write(wall);
      redo_write(w->x&0xffff);
      redo_write((w->x>>16)&0xffff);
      redo_write(w->y&0xffff);
      redo_write((w->y>>16)&0xffff);

      RvR_port_wall_move(map,wall,x,y);
   }
}

static void redo_move_wall(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint32_t y = undo_buffer[pos];  pos = WRAP(pos+1);
      y = (y<<16)+undo_buffer[pos];    pos = WRAP(pos+1);
      uint32_t x = undo_buffer[pos];  pos = WRAP(pos+1);
      x = (x<<16)+undo_buffer[pos];    pos = WRAP(pos+1);
      int16_t wall = undo_buffer[pos]; pos = WRAP(pos+1);

      RvR_port_wall *w = map->walls+wall;
      undo_write(wall);
      undo_write(w->x&0xffff);
      undo_write((w->x>>16)&0xffff);
      undo_write(w->y&0xffff);
      undo_write((w->y>>16)&0xffff);

      RvR_port_wall_move(map,wall,x,y);
   }
}

static void undo_sector(int pos, int endpos)
{
   /*while(pos!=endpos)
   {
      int16_t sector = undo_buffer[pos]; pos = WRAP(pos-1);

      //Write all walls
      redo_write(w->x&0xffff);
      redo_write((w->x>>16)&0xffff);
      redo_write(w->y&0xffff);
      redo_write((w->y>>16)&0xffff);
      for(int i = map->sectors[sector].wall_count-1;i>=0;i--)
      {
      }*/

      /*RvR_port_wall *w = map->walls+wall;
      redo_write(wall);
      redo_write(w->x&0xffff);
      redo_write((w->x>>16)&0xffff);
      redo_write(w->y&0xffff);
      redo_write((w->y>>16)&0xffff);

      RvR_port_wall_move(map,wall,x,y);*/
   //}
}

static void redo_sector(int pos, int endpos)
{
}
//-------------------------------------
