/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "collision.h"
#include "map.h"
#include "grid.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void collision_movex(Entity *e);
static void collision_movey(Entity *e);
static void collision_movez(Entity *e, RvR_fix16 *floor_height, RvR_fix16 *ceiling_height);

static void collision_project(RvR_fix16 vx, RvR_fix16 vy, RvR_fix16 bx, RvR_fix16 by, RvR_fix16 *ox, RvR_fix16 *oy);
//-------------------------------------

//Function implementations

void collision_move(Entity *e, RvR_fix16 *floor_height, RvR_fix16 *ceiling_height)
{
   if(e->removed)
      return;

   collision_movex(e);
   collision_movey(e);
   collision_movez(e,floor_height,ceiling_height);

   //Lower velocity/friction
   e->vx = RvR_fix16_mul(e->vx,54784);
   e->vy = RvR_fix16_mul(e->vy,54784);
   RvR_fix16 vel_mag = RvR_fix16_sqrt(RvR_fix16_mul(e->vx,e->vx)+RvR_fix16_mul(e->vy,e->vy));
   if(vel_mag<4096)
      e->vx = e->vy = 0;
}

static void collision_movex(Entity *e)
{
   if(e==NULL||e->removed)
      return;

   //No velocity, no movement
   if(e->vx/48==0)
      return 0;

   RvR_fix16 newx = e->x+e->vx/48;
   RvR_fix16 newy = e->y;
   RvR_fix16 nvx = e->vx;
   RvR_fix16 nvy = e->vy;

   RvR_fix16 bottom_limit = e->z;
   RvR_fix16 top_limit = e->z+e->col_height;

   if(e->vz<=0)
      bottom_limit+=CAMERA_COLL_STEP_HEIGHT;

   //Map collision
   if(e->vx>=0)
   {
      int left = (e->x+e->col_radius)/65536;
      int right = (newx+e->col_radius)/65536;
      int top = (newy-e->col_radius)/65536;
      int bottom = (newy+e->col_radius)/65536;
      int middlex = (e->x)/65536;

      for(int y = top;y<=bottom;y++)
      {
         for(int x = middlex;x<=right;x++)
         {
            //Sprite collision
            Grid_square *s = grid_square(x,y);
            if(s!=NULL)
            {
               Grid_entity *ge = s->entities;
               for(;ge!=NULL;ge = ge->next)
               {
                  //Itself
                  if(ge->ent==e)
                     continue;

                  //Removed
                  if(ge->ent->removed)
                     continue;

                  //0 radius
                  if(ge->ent->col_radius==0)
                     continue;

                  //no z overlap
                  if(ge->ent->z>e->z+e->col_height||ge->ent->z+ge->ent->col_height<e->z)
                     continue;

                  //No y overlap
                  RvR_fix16 dy = RvR_abs(ge->ent->y-e->y);
                  RvR_fix16 tot_rad = ge->ent->col_radius+e->col_radius;
                  if(dy>=tot_rad)
                     continue;

                  //Behind old pos
                  if(ge->ent->x<e->x)
                     continue;

                  //Potential collision
                  RvR_fix16 cx = ge->ent->x-RvR_fix16_sqrt(RvR_fix16_mul(tot_rad,tot_rad)-RvR_fix16_mul(dy,dy))-1;
                  if(cx<=newx)
                     collision_project(e->vx,e->vy,-(ge->ent->y-e->y),ge->ent->x-e->x,NULL,&nvy);
                  newx = RvR_min(newx,cx);
               }
            }

            if(x<left+1)
               continue;

            RvR_fix16 floor = RvR_ray_map_floor_height_at(map_current(),x,y);
            RvR_fix16 ceiling = RvR_ray_map_ceiling_height_at(map_current(),x,y);

            //Collision
            RvR_fix16 cx = x*65536-e->col_radius-1;
            if((floor>bottom_limit||ceiling<top_limit)&&cx<newx)
            {
               newx = cx;
               nvx = 0;
               nvy = e->vy;
            }
         }
      }

      //Don't move farther than original position
      if(newx>=e->x)
      {
         e->vx = nvx;
         e->vy = nvy;
      }
      newx = RvR_max(newx,e->x);
   }
   else
   {
      int left = (newx-e->col_radius)/65536;
      int right = (e->x-e->col_radius)/65536;
      int top = (newy-e->col_radius)/65536;
      int bottom = (newy+e->col_radius)/65536;
      int middlex = (e->x)/65536;

      for(int y = top;y<=bottom;y++)
      {
         for(int x = left;x<=middlex;x++)
         //for(int x = left;x<right;x++)
         {
            //Sprite collision
            Grid_square *s = grid_square(x,y);
            if(s!=NULL)
            {
               Grid_entity *ge = s->entities;
               for(;ge!=NULL;ge = ge->next)
               {
                  //Itself
                  if(ge->ent==e)
                     continue;

                  //Removed
                  if(ge->ent->removed)
                     continue;

                  //0 radius
                  if(ge->ent->col_radius==0)
                     continue;

                  //no z overlap
                  if(ge->ent->z>e->z+e->col_height||ge->ent->z+ge->ent->col_height<e->z)
                     continue;

                  //No y overlap
                  RvR_fix16 dy = RvR_abs(ge->ent->y-e->y);
                  RvR_fix16 tot_rad = ge->ent->col_radius+e->col_radius;
                  if(dy>=tot_rad)
                     continue;

                  //Infront of old pos
                  if(ge->ent->x>e->x)
                     continue;

                  //Potential collision
                  RvR_fix16 cx = ge->ent->x+RvR_fix16_sqrt(RvR_fix16_mul(tot_rad,tot_rad)-RvR_fix16_mul(dy,dy))+1;
                  if(cx>=newx)
                     collision_project(e->vx,e->vy,-(ge->ent->y-e->y),ge->ent->x-e->x,NULL,&nvy);
                  newx = RvR_max(newx,cx);
                  //newx = RvR_max(newx,ge->ent->x+RvR_fix16_sqrt(RvR_fix16_mul(tot_rad,tot_rad)-RvR_fix16_mul(dy,dy))+1);
               }
            }

            if(x>=right)
               continue;

            RvR_fix16 floor = RvR_ray_map_floor_height_at(map_current(),x,y);
            RvR_fix16 ceiling = RvR_ray_map_ceiling_height_at(map_current(),x,y);

            //Collision
            RvR_fix16 cx = x*65536+65536+e->col_radius+1;
            if((floor>bottom_limit||ceiling<top_limit)&&cx>=newx)
            {
               newx = cx;
               nvx = 0;
               nvy = e->vy;
               //newx = RvR_max(newx,x*65536+65537+e->col_radius);
            }
         }
      }

      //Don't move farther than original position
      if(newx<=e->x)
      {
         e->vx = nvx;
         e->vy = nvy;
      }
      newx = RvR_min(newx,e->x);
   }

   grid_entity_update_pos(e,newx,newy,e->z);
}

static void collision_movey(Entity *e)
{
   if(e==NULL||e->removed)
      return;

   //No velocity, no movement
   if(e->vy/48==0)
      return 0;

   RvR_fix16 newx = e->x;
   RvR_fix16 newy = e->y+e->vy/48;
   RvR_fix16 nvx = e->vx;
   RvR_fix16 nvy = e->vy;

   RvR_fix16 bottom_limit = e->z;
   RvR_fix16 top_limit = e->z+e->col_height;

   if(e->vz<=0)
      bottom_limit+=CAMERA_COLL_STEP_HEIGHT;

   //Map collision
   if(e->vy>=0)
   {
      int left = (newx-e->col_radius)/65536;
      int right = (newx+e->col_radius)/65536;
      int top = (e->y+e->col_radius)/65536;
      int bottom = (newy+e->col_radius)/65536;
      int middley = (e->y)/65536;

      for(int y = middley;y<=bottom;y++)
      //for(int y = top+1;y<=bottom;y++)
      {
         for(int x = left;x<=right;x++)
         {
            //Sprite collision
            Grid_square *s = grid_square(x,y);
            if(s!=NULL)
            {
               Grid_entity *ge = s->entities;
               for(;ge!=NULL;ge = ge->next)
               {
                  //Itself
                  if(ge->ent==e)
                     continue;

                  //Removed
                  if(ge->ent->removed)
                     continue;

                  //0 radius
                  if(ge->ent->col_radius==0)
                     continue;

                  //no z overlap
                  if(ge->ent->z>e->z+e->col_height||ge->ent->z+ge->ent->col_height<e->z)
                     continue;

                  //No y overlap
                  RvR_fix16 dx = RvR_abs(ge->ent->x-e->x);
                  RvR_fix16 tot_rad = ge->ent->col_radius+e->col_radius;
                  if(dx>=tot_rad)
                     continue;

                  //Behind old pos
                  if(ge->ent->y<e->y)
                     continue;

                  //Potential collision
                  RvR_fix16 cy = ge->ent->y-RvR_fix16_sqrt(RvR_fix16_mul(tot_rad,tot_rad)-RvR_fix16_mul(dx,dx))-1;
                  if(cy<=newy)
                     collision_project(e->vx,e->vy,-(ge->ent->y-e->y),ge->ent->x-e->x,&nvx,NULL);
                  newy = RvR_min(newy,cy);
                  //newy = RvR_min(newy,ge->ent->y-RvR_fix16_sqrt(RvR_fix16_mul(tot_rad,tot_rad)-RvR_fix16_mul(dx,dx))-1);
               }
            }

            if(y<top+1)
               continue;

            RvR_fix16 floor = RvR_ray_map_floor_height_at(map_current(),x,y);
            RvR_fix16 ceiling = RvR_ray_map_ceiling_height_at(map_current(),x,y);

            //Collision
            RvR_fix16 cy = y*65536-e->col_radius-1;
            if((floor>bottom_limit||ceiling<top_limit)&&cy<=newy)
            {
               newy = cy;
               nvx = e->vx;
               nvy = 0;
            }
         }
      }

      //Don't move farther than original position
      if(newy>=e->y)
      {
         e->vx = nvx;
         e->vy = nvy;
      }
      newy = RvR_max(newy,e->y);
   }
   else
   {
      int left = (newx-e->col_radius)/65536;
      int right = (newx+e->col_radius)/65536;
      int top = (newy-e->col_radius)/65536;
      int bottom = (e->y-e->col_radius)/65536;
      int middley = (e->y)/65536;

      for(int y = top;y<=middley;y++)
      //for(int y = top;y<bottom;y++)
      {
         for(int x = left;x<=right;x++)
         {
            //Sprite collision
            Grid_square *s = grid_square(x,y);
            if(s!=NULL)
            {
               Grid_entity *ge = s->entities;
               for(;ge!=NULL;ge = ge->next)
               {
                  //Itself
                  if(ge->ent==e)
                     continue;

                  //Removed
                  if(ge->ent->removed)
                     continue;

                  //0 radius
                  if(ge->ent->col_radius==0)
                     continue;

                  //no z overlap
                  if(ge->ent->z>e->z+e->col_height||ge->ent->z+ge->ent->col_height<e->z)
                     continue;

                  //No y overlap
                  RvR_fix16 dx = RvR_abs(ge->ent->x-e->x);
                  RvR_fix16 tot_rad = ge->ent->col_radius+e->col_radius;
                  if(dx>=tot_rad)
                     continue;

                  //Infront of old pos
                  if(ge->ent->y>e->y)
                     continue;

                  //Potential collision
                  RvR_fix16 cy = ge->ent->y+RvR_fix16_sqrt(RvR_fix16_mul(tot_rad,tot_rad)-RvR_fix16_mul(dx,dx))+1;
                  if(cy>=newy)
                     collision_project(e->vx,e->vy,-(ge->ent->y-e->y),ge->ent->x-e->x,&nvx,NULL);
                  newy = RvR_max(newy,cy);
               }
            }
            
            if(y>=bottom)
               continue;

            RvR_fix16 floor = RvR_ray_map_floor_height_at(map_current(),x,y);
            RvR_fix16 ceiling = RvR_ray_map_ceiling_height_at(map_current(),x,y);

            //Collision
            RvR_fix16 cy= y*65536+65536+e->col_radius+1;
            if((floor>bottom_limit||ceiling<top_limit)&&cy>=newy)
            {
               newy = cy;
               nvx = e->vx;
               nvy = 0;
            }
         }
      }

      //Don't move farther than original position
      if(newy<=e->y)
      {
         e->vx = nvx;
         e->vy = nvy;
      }
      newy = RvR_min(newy,e->y);
   }

   grid_entity_update_pos(e,newx,newy,e->z);
}

static void collision_movez(Entity *e, RvR_fix16 *floor_height, RvR_fix16 *ceiling_height)
{
   if(e==NULL||e->removed)
      return;

   int left = (e->x-e->col_radius)/65536;
   int right = (e->x+e->col_radius)/65536;
   int top = (e->y-e->col_radius)/65536;
   int bottom = (e->y+e->col_radius)/65536;
   RvR_fix16 floor_max = INT32_MIN;
   RvR_fix16 ceiling_min = INT32_MAX;

   RvR_fix16 newz = e->z+e->vz/64;

   for(int y = top;y<=bottom;y++)
   {
      for(int x = left;x<=right;x++)
      {
         //Sprite collision
         Grid_square *s = grid_square(x,y);
         if(s!=NULL)
         {
            Grid_entity *ge = s->entities;
            for(;ge!=NULL;ge = ge->next)
            {
               //Itself
               if(ge->ent==e)
                  continue;

               //Removed
               if(ge->ent->removed)
                  continue;

               //0 radius
               if(ge->ent->col_radius==0)
                  continue;

               //no z overlap
               if(ge->ent->z>newz+e->col_height||ge->ent->z+ge->ent->col_height<newz)
                  continue;

               //No overlap
               RvR_fix16 dx = RvR_abs(ge->ent->x-e->x);
               RvR_fix16 dy = RvR_abs(ge->ent->y-e->y);
               RvR_fix16 tot_rad = ge->ent->col_radius+e->col_radius;
               if(RvR_fix16_mul(dx,dx)+RvR_fix16_mul(dy,dy)>=RvR_fix16_mul(tot_rad,tot_rad))
                  continue;

               RvR_fix16 depth_z;
               if(newz>ge->ent->z)
                  depth_z = (ge->ent->z+ge->ent->col_height)-(newz);
               else
                  depth_z = -((newz+e->col_height)-ge->ent->z);

               if(depth_z>0&&ge->ent->z+ge->ent->col_height>floor_max)
                  floor_max = ge->ent->z+ge->ent->col_height+1;
               else if(depth_z<0&&ge->ent->z<ceiling_min)
                  ceiling_min = ge->ent->z-1;
            }
         }

         RvR_fix16 floor = RvR_ray_map_floor_height_at(map_current(),x,y);
         RvR_fix16 ceiling = RvR_ray_map_ceiling_height_at(map_current(),x,y);
         
         if(floor>floor_max)
            floor_max = floor;
         if(ceiling<ceiling_min)
            ceiling_min = ceiling;
      }
   }

   *floor_height = floor_max;
   *ceiling_height = ceiling_min;

   e->z = RvR_min(ceiling_min-e->col_height,RvR_max(e->z+e->vz/64,floor_max));

   //No need to update grid here
   //grid_entity_update_pos(e,newx,newy,e->z);
}

static void collision_project(RvR_fix16 vx, RvR_fix16 vy, RvR_fix16 bx, RvR_fix16 by, RvR_fix16 *ox, RvR_fix16 *oy)
{
   RvR_fix16 b2 = RvR_fix16_mul(bx,bx)+RvR_fix16_mul(by,by);
   RvR_fix16 ab = RvR_fix16_mul(vx,bx)+RvR_fix16_mul(vy,by);
   RvR_fix16 factor = RvR_fix16_div(ab,RvR_non_zero(b2));

   if(ox!=NULL)
      *ox = RvR_fix16_mul(bx,factor);
   if(oy!=NULL)
      *oy = RvR_fix16_mul(by,factor);
}
//-------------------------------------
