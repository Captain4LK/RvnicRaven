/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _GAMESTATE_H_

#define _GAMESTATE_H_

typedef struct Entity Entity;

typedef struct
{
   RvR_fix22 pos[3];
   RvR_fix22 vel[3];
   RvR_fix22 dir;
   int on_ground;
   uint16_t sector;
   RvR_fix22 step_height;
   RvR_fix22 height;
   RvR_fix22 radius; //No collider --> radius = 0
}Collider;

struct Entity
{
   //Entity identification
   uint64_t generation;
   int removed;
   //-------------------------------------

   //Collision
   RvR_fix22 pos[3];
   RvR_fix22 vel[3];
   RvR_fix22 dir;
   int on_ground;
   uint16_t sector;
   RvR_fix22 step_height;
   RvR_fix22 radius;
   RvR_fix22 height;
   RvR_fix22 px;
   RvR_fix22 py;

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

   int shearing;
   RvR_fix22 vis_off_vel;
}Player;

typedef struct
{
   RvR_port_map *map;
   RvR_port_cam cam;
   RvR_port_selection select;

   Entity *entities;
   Player player;
}Gamestate;

Gamestate *gamestate_new();
void gamestate_free(Gamestate *s);

void gamestate_map(Gamestate *s, uint16_t id);

#endif
