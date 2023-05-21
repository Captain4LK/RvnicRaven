/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _GRID_H_

#define _GRID_H_

typedef struct Grid_entity Grid_entity;
typedef struct Grid_card Grid_card;

struct Grid_entity
{
   Entity *ent;
   Grid_entity *next;
   Grid_entity **prev_next;
};

struct Grid_card
{
   Card *card;
   Grid_card *next;
   Grid_card **prev_next;
};

typedef struct
{
   uint8_t blocked;
   Grid_entity *entities;
   Grid_card *cards;
}Grid_square;

typedef struct
{
   int width;
   int height;
   Grid_square *grid;
}Grid;

void grid_build();
Grid_square *grid_square(int x, int y);

void grid_entity_remove(Entity *e);
void grid_entity_add(Entity *e);
void grid_entity_update_pos(Entity *e, RvR_vec3 new_pos);

void grid_card_remove(Card *c);
void grid_card_add(Card *c);
void grid_card_update_pos(Card *c, RvR_vec3 new_pos);

void grid_entity_use(Entity *e);

uint32_t grid_counter_next();

#endif
