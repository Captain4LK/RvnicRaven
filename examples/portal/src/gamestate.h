/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _GAMESTATE_H_

#define _GAMESTATE_H_

typedef struct Entity Entity;

struct Entity
{
   //Entity identification
   uint64_t generation;
   int removed;
   //-------------------------------------

   //Collision
   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
   RvR_fix22 vx;
   RvR_fix22 vy;
   RvR_fix22 vz;
   RvR_fix22 px;
   RvR_fix22 py;
   int on_ground;
   RvR_fix22 direction;
   RvR_fix22 col_height;
   RvR_fix22 col_radius; //col_radius==0 --> no collider

   //bit0: shootable
   uint32_t col_flags;
   //-------------------------------------
   
   //Linked list
   Entity **prev_next;
   Entity *next;
   //-------------------------------------
};

typedef struct
{
   Entity *entity;
}Player;

typedef struct
{
   RvR_port_map *map;
   RvR_port_cam cam;

   Entity *entities;
   Player player;
}Gamestate;

Gamestate *gamestate_new();
void gamestate_free(Gamestate *s);

void gamestate_map(Gamestate *s, uint16_t id);

#endif
