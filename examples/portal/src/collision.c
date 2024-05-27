/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "entity.h"
#include "collision.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint16_t *collision_sector_stack = NULL;
//-------------------------------------

//Function prototypes
static void collision_movex(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy);
static void collision_movey(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy);
//-------------------------------------

//Function implementations

void collision_move(Gamestate *state, Entity *e, RvR_fix22 *floor_height, RvR_fix22 *ceiling_height)
{
   if(e==NULL||e->removed)
      return;

   RvR_fix22 vx = e->vx;
   RvR_fix22 vy = e->vy;
   RvR_fix22 nvx = vx;
   RvR_fix22 nvy = vy;
   RvR_fix22 mag_v = RvR_fix22_sqrt(RvR_fix22_mul(vx/48,vx/48)+RvR_fix22_mul(vy/48,vy/48));
   printf("%d\n",mag_v);

   RvR_fix22 tot_mag = 0;
   for(int i = 0;i<4;i++)
   {
      RvR_fix22 ox = e->x;
      RvR_fix22 oy = e->y;

      nvx = e->vx;
      nvy = e->vy;
      collision_movex(state,e,&nvx,&nvy);
      collision_movey(state,e,&nvx,&nvy);

      RvR_fix22 dx = e->x-ox;
      RvR_fix22 dy = e->y-oy;
      tot_mag+=RvR_fix22_sqrt(RvR_fix22_mul(dx,dx)+RvR_fix22_mul(dy,dy));
      if(tot_mag>=mag_v)
         break;

      RvR_fix22 mag_n = RvR_fix22_sqrt(RvR_fix22_mul(nvx,nvx)+RvR_fix22_mul(nvy,nvy));
      if(mag_n==0)
         break;
      if(mag_n/48+tot_mag>mag_v)
      {
         RvR_fix22 scale = RvR_fix22_div((mag_v-tot_mag)*48,RvR_non_zero(mag_n));
         nvx = RvR_fix22_mul(nvx,scale);
         nvy = RvR_fix22_mul(nvy,scale);
      }

      e->vx = nvx;
      e->vy = nvy;
   }

   //collision_movez
   e->vx = vx;
   e->vy = vy;

   //printf("%d %d\n",vx/48,vy/48);
   entity_update_pos(state,e,e->x+vx/48,e->y+vy/48,e->z);

   e->vx = RvR_fix22_mul(e->vx,856);
   e->vy = RvR_fix22_mul(e->vy,856);
}

static void collision_movex(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy)
{
   RvR_fix22 np[2];
   np[0] = e->x+e->vx/48;
   np[1] = e->y;
   //uint16_t nsector = RvR_port_sector_update(state->map,e->sector,np[0],np[1]);
   //TODO(Captain4LK): don't remove from stack, so that we can check if already added
   //should be faster than using up to 8KB bitmap
   RvR_array_length_set(collision_sector_stack,0);
   RvR_array_push(collision_sector_stack,e->sector);
   for(int i = 0;i<RvR_array_length(collision_sector_stack);i++)
   {
      uint16_t sector = collision_sector_stack[i];

      for(int j = 0;j<state->map->sectors[sector].wall_count;j++)
      {
         RvR_port_wall *wall = state->map->walls+state->map->sectors[sector].wall_first+j;

         RvR_fix22 p0[2];
         RvR_fix22 p1[2];
         p0[0] = wall->x;
         p0[1] = wall->y;
         p1[0] = state->map->walls[wall->p2].x;
         p1[1] = state->map->walls[wall->p2].y;

         RvR_fix22 len2 = RvR_fix22_mul(p1[0]-p0[0],p1[0]-p0[0])+RvR_fix22_mul(p1[1]-p0[1],p1[1]-p0[1]);
         RvR_fix22 t = RvR_fix22_mul(np[0]-p0[0],p1[0]-p0[0])+RvR_fix22_mul(np[1]-p0[1],p1[1]-p0[1]);
         //RvR_fix22 t = (np[0]-p0[0])*(p1[0]-p0[0])+(np[1]-p0[1])*(p1[1]-p0[1]);
         t = RvR_max(0,RvR_min(len2,t));

         RvR_fix22 proj[2];
         proj[0] = p0[0]+RvR_fix22_div(RvR_fix22_mul(t,p1[0]-p0[0]),RvR_non_zero(len2));
         proj[1] = p0[1]+RvR_fix22_div(RvR_fix22_mul(t,p1[1]-p0[1]),RvR_non_zero(len2));
         RvR_fix22 dist2 = RvR_fix22_mul(proj[0]-np[0],proj[0]-np[0])+RvR_fix22_mul(proj[1]-np[1],proj[1]-np[1]);
         if(dist2>RvR_fix22_mul(e->col_radius,e->col_radius))
            continue;

         //Intersection
      }
      //RvR_array_length_set(collision_sector_stack,RvR_array_length(collision_sector_stack)-1);

      //Check all walls in sector for intersection
      //intersection with portal --> add adjacent sector
   }
}

static void collision_movey(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy)
{
}
//-------------------------------------
