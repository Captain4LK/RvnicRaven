/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../RvR/RvnicRaven.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "collision.h"
#include "grid.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void collision_intersects(Entity *a, Entity *b, RvR_fix22 *depth, RvR_vec2 *normal);
//-------------------------------------

//Function implementations

void collision_move(Entity *e, RvR_fix22 *floor_height, RvR_fix22 *ceiling_height)
{
   if(e->removed)
      return;

   int8_t moves_in_plane = (e->vel.x>>6)!=0||(e->vel.y>>6)!=0;

   if(floor_height!=NULL)
      *floor_height = INT32_MIN;
   if(ceiling_height!=NULL)
      *ceiling_height = INT32_MAX;

   Entity *col = entities;
   Entity cur = {0};
   cur.pos.x = e->pos.x+(e->vel.x>>6);
   cur.pos.y = e->pos.y+(e->vel.y>>6);
   cur.pos.z = e->pos.z+e->vel.z;
   cur.col_radius = 256;
   cur.col_height = 768;
   while(col!=NULL)
   {
      if(col==e)
         goto next;
      if(col->removed)
         goto next;
      if(col->col_radius==0)
         goto next;
      if(col->pos.z>(cur.pos.z)+cur.col_height)
         goto next;
      if(col->pos.z+col->col_height<(cur.pos.z))
         goto next;

      RvR_fix22 depth;
      RvR_vec2 normal;

      collision_intersects(&cur,col,&depth,&normal);
      if(depth>0)
      {
         //Compare collision resolution posiblilities
         RvR_fix22 depth_z;
         if(cur.pos.z>col->pos.z)
            depth_z = (col->pos.z+col->col_height)-(cur.pos.z);
         else
            depth_z = -((cur.pos.z+cur.col_height)-col->pos.z);

         if(RvR_abs(depth_z)>depth)
         {
            cur.pos.x-=(normal.x*depth)/1024;
            cur.pos.y-=(normal.y*depth)/1024;
         }
         else
         {
            cur.pos.z+=depth_z;
            if(floor_height!=NULL&&depth_z>0&&col->pos.z+col->col_height>*floor_height)
               *floor_height = col->pos.z+col->col_height;
            else if(ceiling_height!=NULL&&depth_z<0&&col->pos.z<*ceiling_height)
               *ceiling_height = col->pos.z;
         }
      }

next:
      col = col->next;
   }

   RvR_vec3 offset;
   offset.x = cur.pos.x-e->pos.x;
   offset.y = cur.pos.y-e->pos.y;
   offset.z = cur.pos.z-e->pos.z;
   RvR_vec3 old_pos = cur.pos;
   old_pos.x-=offset.x;
   old_pos.y-=offset.y;
   old_pos.z-=offset.z;

   int16_t x_square_new, y_square_new;

   RvR_vec2 corner; // BBox corner in the movement direction
   RvR_vec2 corner_new;

   int16_t x_dir = offset.x>0?1:-1;
   int16_t y_dir = offset.y>0?1:-1;

   corner.x = e->pos.x+x_dir*CAMERA_COLL_RADIUS;
   corner.y = e->pos.y+y_dir*CAMERA_COLL_RADIUS;

   int16_t x_square = RvR_div_round_down(corner.x,1024);
   int16_t y_square = RvR_div_round_down(corner.y,1024);

   corner_new.x = corner.x+offset.x;
   corner_new.y = corner.y+offset.y;

   x_square_new = RvR_div_round_down(corner_new.x,1024);
   y_square_new = RvR_div_round_down(corner_new.y,1024);

   RvR_fix22 bottom_limit = -RvR_fix22_infinity;
   RvR_fix22 top_limit = RvR_fix22_infinity;

   RvR_fix22 curr_ceil_height = RvR_fix22_infinity;


   int can_step = 0;
   int did_step = 0;
   if(e->vel.z<=0)
      can_step = 1;

   bottom_limit = e->pos.z;
   top_limit = e->pos.z+e->col_height;

   curr_ceil_height = RvR_ray_map_ceiling_height_at(x_square,y_square);

   // checks a single square for collision against the camera
#define collCheck(dir,s1,s2)\
   RvR_fix22 height = RvR_ray_map_floor_height_at(s1,s2);\
   if(height>bottom_limit||curr_ceil_height-height<e->col_height)\
   { \
      dir##_collides = 1;\
      if(can_step&&height<=bottom_limit+CAMERA_COLL_STEP_HEIGHT&&!(curr_ceil_height-height<e->col_height)) \
      { \
         dir##_collides = 0; \
         did_step = 1; \
      } \
   } \
   {\
      RvR_fix22 height2 = RvR_ray_map_ceiling_height_at(s1,s2);\
      if((height2<top_limit)||((height2-height)<\
      (e->col_height)))\
         dir##_collides = 1;\
   }

   // check collision against non-diagonal square
#define collCheckOrtho(dir,dir2,s1,s2,x)\
   if (dir##_square_new != dir##_square)\
   {\
      collCheck(dir,s1,s2)\
   }\
   if(!dir##_collides)\
   { /* now also check for coll on the neighbouring square */ \
      int16_t dir2##_square2 = RvR_div_round_down(corner.dir2-dir2##_dir *\
      CAMERA_COLL_RADIUS*2,1024);\
      if(dir2##_square2!=dir2##_square)\
      {\
         if(x)\
         {\
            collCheck(dir,dir##_square_new,dir2##_square2)\
         }\
         else\
         {\
            collCheck(dir,dir2##_square2,dir##_square_new)\
         }\
      }\
   }

   int8_t x_collides = 0;
   int8_t y_collides = 0;
   int8_t xy_collides = 0;
   collCheckOrtho(x,y,x_square_new,y_square,1)
   collCheckOrtho(y,x,x_square,y_square_new,0)

   if(x_collides||y_collides)
   {
      if(moves_in_plane)
      {
         #define collHandle(dir)\
         if (dir##_collides)\
            corner_new.dir = (dir##_square)*1024+\
            1024/2+dir##_dir*(1024/2)-\
            dir##_dir;\

         collHandle(x)
         collHandle(y)

      }
      else
      {
         /* Player collides without moving in the plane; this can happen e.g. on
         elevators due to vertical only movement. This code can get executed
         when force == 1. */

         RvR_vec2 square_pos;
         RvR_vec2 new_pos;

         square_pos.x = x_square*1024;
         square_pos.y = y_square*1024;

         new_pos.x = RvR_max(square_pos.x+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.x+1024-CAMERA_COLL_RADIUS-1,e->pos.x));
         new_pos.y = RvR_max(square_pos.y+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.y+1024-CAMERA_COLL_RADIUS-1,e->pos.y));

         corner_new.x = corner.x+(new_pos.x-e->pos.x);
         corner_new.y = corner.y+(new_pos.y-e->pos.y);
      }
   }
   else 
   {
      /* If no non-diagonal collision is detected, a diagonal/corner collision
      can still happen, check it here. */

      if(x_square!=x_square_new&&y_square!=y_square_new)
      {
         collCheck(xy,x_square_new,y_square_new)

         if (xy_collides)
         {
            corner_new = corner;
         }
      }
   }

   RvR_vec3 new_pos;
   new_pos.x = corner_new.x-x_dir*CAMERA_COLL_RADIUS;
   new_pos.y = corner_new.y-y_dir*CAMERA_COLL_RADIUS;
   new_pos.z = e->pos.z+offset.z;
   grid_entity_update_pos(e,new_pos);

   int16_t x_square1 = RvR_div_round_down(e->pos.x-CAMERA_COLL_RADIUS,1024);

   int16_t x_square2 = RvR_div_round_down(e->pos.x+CAMERA_COLL_RADIUS,1024);

   int16_t y_square1 = RvR_div_round_down(e->pos.y-CAMERA_COLL_RADIUS,1024);

   int16_t y_square2 = RvR_div_round_down(e->pos.y+CAMERA_COLL_RADIUS,1024);

   bottom_limit = RvR_ray_map_floor_height_at(x_square1,y_square1);
   top_limit = RvR_ray_map_ceiling_height_at(x_square1,y_square1);

   RvR_fix22 height;

#define checkSquares(s1,s2)\
   {\
      height = RvR_ray_map_floor_height_at(x_square##s1,y_square##s2);\
      bottom_limit = RvR_max(bottom_limit,height);\
      height = RvR_ray_map_ceiling_height_at(x_square##s1,y_square##s2);\
      top_limit = RvR_min(top_limit,height);\
   }

   if(x_square2!=x_square1)
      checkSquares(2,1)

   if(y_square2!=y_square1)
      checkSquares(1,2)

   if(x_square2!=x_square1&&y_square2!=y_square1)
      checkSquares(2,2)

   if(floor_height!=NULL&&*floor_height<bottom_limit)
      *floor_height = bottom_limit;
   if(ceiling_height!=NULL&&*ceiling_height>top_limit)
      *ceiling_height = top_limit;

   if(e->pos.z<bottom_limit&&did_step)
      e->vis_zoff = RvR_min(e->pos.z-bottom_limit,e->vis_zoff);

   e->pos.z = RvR_clamp(e->pos.z,bottom_limit,top_limit-e->col_height);

#undef checkSquares
#undef collCheckOrtho
#undef collHandle
#undef collCheck

   if((e->vel.x>>6)!=(e->pos.x-old_pos.x))
      e->vel.x = (e->pos.x-old_pos.x)<<6;
   if((e->vel.y>>6)!=(e->pos.y-old_pos.y))
      e->vel.y = (e->pos.y-old_pos.y)<<6;

   //Lower velocity/friction
   e->vel.x = (e->vel.x*900)/1024;
   e->vel.y = (e->vel.y*900)/1024;
   RvR_fix22 vel_mag = (e->vel.x*e->vel.x+e->vel.y*e->vel.y)/1024;
   if(vel_mag<128)
      e->vel.x = e->vel.y = 0;
}

static void collision_intersects(Entity *a, Entity *b, RvR_fix22 *depth, RvR_vec2 *normal)
{
   normal->x = 0;
   normal->y = 0;
   *depth = 0;

   if(a->removed||b->removed)
      return;

   RvR_fix22 dx = b->pos.x-a->pos.x;
   RvR_fix22 dy = b->pos.y-a->pos.y;

   //Early out, overflow protection
   if(RvR_abs(dx)>a->col_radius+b->col_radius)
      return;
   if(RvR_abs(dy)>a->col_radius+b->col_radius)
      return;

   RvR_fix22 depth2 = (dx*dx+dy*dy)/1024;
   RvR_fix22 r = a->col_radius+b->col_radius;
   if(depth2<r)
   {
      *depth = RvR_fix22_sqrt(depth2);
      if(*depth==0)
      {
         normal->x = 0;
         normal->y = 1024;
      }
      else
      {
         normal->x = (dx*1024)/(*depth);
         normal->y = (dy*1024)/(*depth);
      }
      *depth = r-*depth;
   }
}
//-------------------------------------
