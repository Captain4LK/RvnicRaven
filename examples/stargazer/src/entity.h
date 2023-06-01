/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _ENTITY_H_

#define _ENTITY_H_

typedef struct Entity Entity;

struct Entity
{
   //Inventory
   unsigned cards_size;
   Card *cards;
   //-------------------------------------

   //Entity identification
   uint32_t generation;
   int removed;
   //-------------------------------------

   //AI
   uint32_t ai_type;
   void *ai_data;
   //-------------------------------------

   //Grid
   uint32_t grid_counter;
   //-------------------------------------

   //Collision
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   RvR_fix16 vx;
   RvR_fix16 vy;
   RvR_fix16 vz;
   int on_ground;
   RvR_fix16 direction;
   RvR_fix16 col_height;
   RvR_fix16 col_radius; //col_radius==0 --> no collider

   //bit0: shootable
   uint32_t col_flags;
   //-------------------------------------

   //Drawing
   RvR_fix16 vis_zoff;
   int32_t sprite;
   //-------------------------------------

   //Linked list
   Entity **prev_next;
   Entity *next;
   //-------------------------------------
};

typedef struct
{
   uint32_t generation;
   Entity *index;
}Gendex;

extern Entity *entities;

Entity *entity_new();
void entity_free(Entity *e);
void entity_add(Entity *e);
void entity_remove(Entity *e);

unsigned entity_health(const Entity *e);

Gendex gendex_get(Entity *e);
Entity *gendex_try(Gendex gen);

#endif
