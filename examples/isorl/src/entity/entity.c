/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "world_defs.h"
#include "entity.h"
#include "entity_documented.h"
#include "area.h"
#include "tile.h"
#include "ai.h"
#include "log.h"
#include "body.h"
#include "item.h"

#include "player.h"
#include "sprite.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Entity *entity_pool;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Entity *entity_new(World *w)
{
   if(entity_pool == NULL)
   {
      Entity *ne = RvR_malloc(sizeof(*ne) * 256, "entity pool");
      memset(ne, 0, sizeof(*ne) * 256);

      for(int i = 0; i < 256; i++)
         ne[i].id = UINT64_MAX;
      for(int i = 0; i < 256 - 1; i++)
         ne[i].next = &ne[i + 1];
      entity_pool = &ne[0];
   }

   Entity *n = entity_pool;
   entity_pool = n->next;

   uint64_t id = w->next_eid++;
   memset(n, 0, sizeof(*n));
   n->action.id = ACTION_INVALID;
   n->next = NULL;
   n->prev_next = NULL;
   n->g_next = NULL;
   n->g_prev_next = NULL;
   n->id = id;

   return n;
}

void entity_free(Entity *e)
{
   if(e == NULL)
      return;

   entity_grid_remove(e);
   e->id = UINT64_MAX;

   *e->prev_next = e->next;
   if(e->next != NULL)
      e->next->prev_next = e->prev_next;

   e->next = entity_pool;
   entity_pool = e;
}

void entity_add(Area *a, Entity *e)
{
   if(e==NULL)
      return;
   if(a==NULL)
      return;

   int cx = (e->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (e->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (e->pos.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   if(cx<0||cy<0||cz<0)
      return;
   if(cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
      return;

   e->prev_next = &a->chunks[c]->entities;
   if(a->chunks[c]->entities!=NULL)
      a->chunks[c]->entities->prev_next = &e->next;
   e->next = a->chunks[c]->entities;
   a->chunks[c]->entities = e;
}

void entity_grid_add(Area *a, Entity *e)
{
   if(e==NULL)
      return;
   if(a==NULL)
      return;

   int cx = (e->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (e->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (e->pos.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   if(cx<0||cy<0||cz<0)
      return;
   if(cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
      return;

   //if(e->pos.x<0||e->pos.y<0||e->pos.z<0)
      //return;
   //if(e->pos.x>=AREA_DIM * 32||e->pos.y>=AREA_DIM * 32||e->pos.z>=AREA_DIM * 32)
      //return;

   int gx = (e->pos.x-(e->pos.x/32)*32) / 8;
   int gy = (e->pos.y-(e->pos.y/32)*32) / 8;
   int gz = (e->pos.z-(e->pos.z/32)*32) / 8;
   //int gy = (e->pos.y / 8;
   //int gz = (e->pos.z / 8;
   int g = gz*4*4+gy*4+gx;

   e->g_prev_next = &a->chunks[c]->entity_grid[g];
   if(a->chunks[c]->entity_grid[g]!=NULL)
      a->chunks[c]->entity_grid[g]->g_prev_next = &e->g_next;
   e->g_next = a->chunks[c]->entity_grid[g];
   a->chunks[c]->entity_grid[g] = e;

   //size_t g_index = gz * (AREA_DIM * 4) * (AREA_DIM * 4) + gy * (AREA_DIM * 4) + gx;
   //e->g_prev_next = &a->entity_grid[g_index];
   //if(a->entity_grid[g_index]!=NULL)
      //a->entity_grid[g_index]->g_prev_next = &e->g_next;
   //e->g_next = a->entity_grid[g_index];
   //a->entity_grid[g_index] = e;
}

void entity_update_pos(Area *a, Entity *e, Point pos)
{
   if(e==NULL)
      return;
   if(a==NULL)
      return;

   int cx = (e->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (e->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (e->pos.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   if(cx<0||cy<0||cz<0)
      return;
   if(cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
      return;

   //if(new_pos.x<0||new_pos.y<0||new_pos.z<0)
      //return;
   //if(new_pos.x>=AREA_DIM * 32||new_pos.y>=AREA_DIM * 32||new_pos.z>=AREA_DIM * 32)
      //return;

   entity_grid_remove(e);
   e->pos = pos;
   entity_grid_add(a, e);
}

void entity_grid_remove(Entity *e)
{
   if(e==NULL)
      return;

   if(e->g_prev_next==NULL)
      return;

   *e->g_prev_next = e->g_next;
   if(e->g_next != NULL)
      e->g_next->g_prev_next = e->g_prev_next;
}

int entity_pos_valid(Area *a, Entity *e, Point pos)
{
   if(e==NULL)
      return 0;

   int cx = (e->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (e->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (e->pos.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   if(cx<0||cy<0||cz<0)
      return 0;
   if(cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
      return 0;

   //if(pos.x<0||pos.y<0||pos.z<0)
      //return 0;

   //if(pos.x>=AREA_DIM * 32||pos.y>=AREA_DIM * 32||pos.z>=AREA_DIM * 32)
      //return 0;

   uint32_t block = area_tile(a, pos);
   uint32_t floor = area_tile(a, point(pos.x, pos.y, pos.z + 1));
   if(area_entity_at(a, pos, e)!=NULL)
      return 0;
   if(!tile_has_wall(block)&&(tile_has_floor(floor)||tile_is_slope(floor)))
      return 1;
   return 0;
}

unsigned entity_try_move(World *w, Area *a, Entity *e, uint8_t dir)
{
   if(e == NULL)
      return 0;

   //Leave map
   //Update DocEnt if entity is documented
   Point n = point_add_dir(e->pos, dir);
   int cx = (n.x/32)-a->cx+AREA_DIM/2;
   int cy = (n.y/32)-a->cy+AREA_DIM/2;
   int cz = (n.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   //int cx = (n.x/32)-a->cx+AREA_DIM/2;
   //int cy = (n.y/32)-a->cy+AREA_DIM/2;
   //int cz = (n.z/32)-a->cz+AREA_DIM/2;
   //int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   printf("%d %d %d %d %d %d\n",n.x,n.y,n.z,a->cx,a->cy,a->cz);
   if(cx<0||cy<0||cz<0||cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
   //if(n.x<0||n.y<0||n.x>=AREA_DIM * 32||n.y>=AREA_DIM * 32)
   {
      if(entity_is_docent(e))
      {
         e->pos = point_add_dir(e->pos, dir);
         docent_from_entity(w, a, e);
         e->pos = point_sub_dir(e->pos, dir);
      }

      entity_free(e);
      return 2;
   }

   if(entity_pos_valid(a, e, n))
   {
      entity_update_pos(a, e, n);
      return 1;
   }

   //Moving up slopes
   if(tile_is_slope(area_tile(a, e->pos))&&
      tile_has_wall(area_tile(a, n))&&
      entity_pos_valid(a, e, point(n.x, n.y, n.z - 1)))
   {
      entity_update_pos(a, e, point(n.x, n.y, n.z - 1));
      return 1;
   }

   //Moving down slopes
   if(tile_is_slope(area_tile(a, point(e->pos.x, e->pos.y, e->pos.z + 1)))&&
      entity_pos_valid(a, e, point(n.x, n.y, n.z + 1))&&
      !tile_has_wall(area_tile(a, n)))
   {
      entity_update_pos(a, e, point(n.x, n.y, n.z + 1));
      return 1;
   }

   return 0;
}

unsigned entity_try_ascend(Area *a, Entity *e)
{
   if(e==NULL)
      return 0;

   if(entity_pos_valid(a, e, point(e->pos.x, e->pos.y, e->pos.z - 1))&&tile_is_slope(area_tile(a, e->pos)))
   {
      entity_update_pos(a, e, point(e->pos.x, e->pos.y, e->pos.z - 1));
      return 1;
   }

   return 0;
}

unsigned entity_try_descend(Area *a, Entity *e)
{
   if(e==NULL)
      return 0;

   if(entity_pos_valid(a, e, point(e->pos.x, e->pos.y, e->pos.z + 1))&&tile_is_slope(area_tile(a, point(e->pos.x, e->pos.y, e->pos.z + 1))))
   {
      entity_update_pos(a, e, point(e->pos.x, e->pos.y, e->pos.z + 1));
      return 1;
   }

   return 0;
}

void entity_think(World *w, Area *a, Entity *e)
{
   ai_think(w, a, e);
}

//TODO(Captain4LK): kill if dead
void entity_turn(World *w, Area *a, Entity *e)
{
   e->fatigue_next--;
   e->hunger_next--;

   if(e->fatigue_next<=0)
   {
      e->fatigue++;
      e->fatigue_next = 300;
   }

   if(e->hunger_next<=0)
   {
      e->hunger++;
      e->hunger_next = 300;
   }

}

int entity_move_cost(Entity *e)
{
   return 184;
}

void entity_from_def(Entity *e, const EntityDef *def, int female)
{
   if(female)
      body_from_def(&e->body, def->female_body);
   else
      body_from_def(&e->body, def->male_body);
}

void entity_sprite_create(Entity *e)
{
   e->sprite = sprite_new(e);
   sprite_clear(e->sprite, 255);

   if(e->body.def==NULL)
   {
      sprite_draw_sprite(e->sprite, 16384, 0, 0, 0, 0, 32, 32);
      return;
   }
   uint16_t sheet = e->body.def->sprite_sheet;
   for(int i = 0; i<e->body.part_count; i++)
   {
      const Bodypart *bp = &e->body.parts[i];
      int16_t index = bp->def->sprite_index;
      if(index<0)
         continue;
      sprite_draw_sprite(e->sprite, sheet, 0, 0, index * 32, 0, 32, 36);
   }
}

void entity_hit(Entity *e, Entity *src, Item *weapon, int16_t body_part)
{
   if(e==NULL)
      return;

   if(body_part==-1)
   {
      int part_count = 0;
      for(int i = 0; i<e->body.part_count; i++)
         if(e->body.parts[i].hp>0)
            part_count++;
   }

   //if(body_part<0)
   //return;

   e->body.parts[0].hp -= 20;
   puts("DAMAGE");
}

void entity_die(World *w, Area *a, Entity *e)
{
}

int entity_store_item(World *w, Area *a, Entity *e, Item *it)
{
   //Search for free slot
   //-------------------------------------

   //Grasp
   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(e->body.parts[i].slots[j].type==ITEM_SLOT_GRASP&&
            e->body.parts[i].slots[j].it==NULL)
         {
            Item *inv = item_duplicate(w, it);
            item_free(it);

            inv->prev_next = &e->body.parts[i].slots[j].it;
            inv->next = e->body.parts[i].slots[j].it;
            e->body.parts[i].slots[j].it = inv;

            return 0;
         }
      }
   }

   //Container slots in equipment
   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         Item *cur = e->body.parts[i].slots[j].it;
         for(; cur!=NULL; cur = cur->next)
         {
            if(!(cur->def->tags & DEF_ITEM_SLOT_CONTAINER))
               continue;

            Item *inv = item_duplicate(w, it);
            item_free(it);

            inv->prev_next = &cur->container.it;
            if(cur->container.it!=NULL)
               cur->container.it->prev_next = &inv->next;
            inv->next = cur->container.it;
            cur->container.it = inv;

            return 0;
         }
      }
   }
   //-------------------------------------

   return 1;
}

int entity_can_equip(World *w, Area *a, Entity *e, Item *it, int check_space)
{
   if(e==NULL)
      return 0;

   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(check_space&&e->body.parts[i].slots[j].it!=NULL)
            continue;

         //Check slot type
         if(!(
               (e->body.parts[i].slots[j].type==ITEM_SLOT_UPPER&&it->def->tags & DEF_ITEM_EQUIP_UPPER)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_LOWER&&it->def->tags & DEF_ITEM_EQUIP_LOWER)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_HEAD&&it->def->tags & DEF_ITEM_EQUIP_HEAD)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_HAND&&it->def->tags & DEF_ITEM_EQUIP_HAND)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_FOOT&&it->def->tags & DEF_ITEM_EQUIP_FOOT)
               ))
            continue;


         //Check slot layer
         if(!(
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_UNDER&&it->def->tags & DEF_ITEM_LAYER_UNDER)||
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_OVER&&it->def->tags & DEF_ITEM_LAYER_OVER)||
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_ARMOR&&it->def->tags & DEF_ITEM_LAYER_ARMOR)||
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_BACK&&it->def->tags & DEF_ITEM_LAYER_BACK)
               ))
            continue;

         //All passed --> can equip
         return 1;
      }
   }

   return 0;
}

void entity_equip(World *w, Area *a, Entity *e, Item *it)
{
   if(e==NULL)
      return;

   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(e->body.parts[i].slots[j].it!=NULL)
            continue;

         //Check slot type
         if(!(
               (e->body.parts[i].slots[j].type==ITEM_SLOT_UPPER&&it->def->tags & DEF_ITEM_EQUIP_UPPER)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_LOWER&&it->def->tags & DEF_ITEM_EQUIP_LOWER)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_HEAD&&it->def->tags & DEF_ITEM_EQUIP_HEAD)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_HAND&&it->def->tags & DEF_ITEM_EQUIP_HAND)||
               (e->body.parts[i].slots[j].type==ITEM_SLOT_FOOT&&it->def->tags & DEF_ITEM_EQUIP_FOOT)
               ))
            continue;


         //Check slot layer
         if(!(
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_UNDER&&it->def->tags & DEF_ITEM_LAYER_UNDER)||
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_OVER&&it->def->tags & DEF_ITEM_LAYER_OVER)||
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_ARMOR&&it->def->tags & DEF_ITEM_LAYER_ARMOR)||
               (e->body.parts[i].slots[j].layer==ITEM_SLOT_BACK&&it->def->tags & DEF_ITEM_LAYER_BACK)
               ))
            continue;

         //All passed --> can equip
         Item *inv = item_duplicate(w, it);
         item_free(it);

         inv->prev_next = &e->body.parts[i].slots[j].it;
         inv->next = e->body.parts[i].slots[j].it;
         e->body.parts[i].slots[j].it = inv;

         return;
      }
   }
}

void entity_remove(World *w, Area *a, Entity *e, Item *it)
{
   //Search for free slot
   Item_slot *hand = NULL;
   int min = INT_MAX;

   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(e->body.parts[i].slots[j].type!=ITEM_SLOT_GRASP)
            continue;

         int len = item_list_length(e->body.parts[i].slots[j].it);
         if(len<min)
         {
            min = len;
            hand = &e->body.parts[i].slots[j];
         }
      }
   }

   //No usable grasp slot
   if(hand==NULL)
      return;

   //Check if item accessible
   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(e->body.parts[i].slots[j].type==ITEM_SLOT_GRASP)
            continue;

         for(Item *cur = e->body.parts[i].slots[j].it; cur!=NULL; cur = cur->next)
         {
            if(cur!=it)
               continue;

            Item *inv = item_duplicate(w, it);
            item_free(it);

            inv->prev_next = &hand->it;
            if(hand->it!=NULL)
               hand->it->prev_next = &inv->next;
            inv->next = hand->it;
            hand->it = inv;

            return;
         }
      }
   }
}

void entity_put(World *w, Area *a, Entity *e, Item *it, Item *container)
{
   if(it==NULL||container==NULL)
      return;

   if(!(container->def->tags & DEF_ITEM_SLOT_CONTAINER))
      return;

   if(it==container)
      return;

   //Check if container accessible
   Item_slot *contain = NULL;
   int cx = (e->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (e->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (e->pos.z/32)-a->cz+AREA_DIM/2;
   int gx = (e->pos.x-(e->pos.x/32)*32) / 8;
   int gy = (e->pos.y-(e->pos.y/32)*32) / 8;
   int gz = (e->pos.z-(e->pos.z/32)*32) / 8;
   //int gx = e->pos.x / 8;
   //int gy = e->pos.y / 8;
   //int gz = e->pos.z / 8;

   for(Item *cur = a->chunks[cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx]->item_grid[gz*4*4+gy*4+gx]; cur!=NULL; cur = cur->g_next)
      if(point_equal(cur->pos, e->pos)&&cur==container)
         contain = &container->container;

   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         for(Item *cur = e->body.parts[i].slots[j].it; cur!=NULL; cur = cur->next)
         {
            if(cur==container)
               contain = &container->container;
         }
      }
   }

   if(contain==NULL)
      return;

   //Check if item accessible
   //gx = e->pos.x / 8;
   //gy = e->pos.y / 8;
   //gz = e->pos.z / 8;
   for(Item *cur = a->chunks[cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx]->item_grid[gz*4*4+gy*4+gx]; cur!=NULL; cur = cur->g_next)
   {
      if(!(point_equal(cur->pos, e->pos)&&cur!=container))
         continue;

      Item *inv = item_duplicate(w, it);
      item_free(it);

      inv->prev_next = &contain->it;
      if(contain->it!=NULL)
         contain->it->prev_next = &inv->next;
      inv->next = contain->it;
      contain->it = inv;

      return;
   }

   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(e->body.parts[i].slots[j].type==ITEM_SLOT_GRASP)
         {
            Item *cur = e->body.parts[i].slots[j].it;
            for(; cur!=NULL; cur = cur->next)
            {
               if(cur==it)
               {
                  Item *inv = item_duplicate(w, it);
                  item_free(it);

                  inv->prev_next = &contain->it;
                  if(contain->it!=NULL)
                     contain->it->prev_next = &inv->next;
                  inv->next = contain->it;
                  contain->it = inv;

                  return;
               }
            }
         }

         for(Item *cur = e->body.parts[i].slots[j].it; cur!=NULL; cur = cur->next)
         {
            if(!(cur->def->tags & DEF_ITEM_SLOT_CONTAINER))
               continue;

            for(Item *con = cur->container.it; con!=NULL; con = con->next)
            {
               if(con==it)
               {
                  Item *inv = item_duplicate(w, it);
                  item_free(it);

                  inv->prev_next = &contain->it;
                  if(contain->it!=NULL)
                     contain->it->prev_next = &inv->next;
                  inv->next = contain->it;
                  contain->it = inv;

                  return;
               }
            }
         }
      }
   }
}
//-------------------------------------
