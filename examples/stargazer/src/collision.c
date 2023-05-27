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
static void collision_intersects(Entity *a, Entity *b, RvR_fix16 *depth, RvR_fix16 *normalx, RvR_fix16 *normaly);
//-------------------------------------

//Function implementations

void collision_move(Entity *e, RvR_fix16 *floor_height, RvR_fix16 *ceiling_height)
{
   if(e->removed)
      return;

   int8_t moves_in_plane = (e->vx)!=0||(e->vy)!=0;

   if(floor_height!=NULL)
      *floor_height = INT32_MIN;
   if(ceiling_height!=NULL)
      *ceiling_height = INT32_MAX;

   Entity *col = entities;
   Entity cur = {0};
   cur.x = e->x+(e->vx);
   cur.y = e->y+(e->vy);
   cur.z = e->z+e->vz;
   cur.col_radius = 256*64;
   cur.col_height = 768*64;
   while(col!=NULL)
   {
      if(col==e)
         goto next;
      if(col->removed)
         goto next;
      if(col->col_radius==0)
         goto next;
      if(col->z>(cur.z)+cur.col_height)
         goto next;
      if(col->z+col->col_height<(cur.z))
         goto next;

      RvR_fix16 depth;
      RvR_fix16 normx;
      RvR_fix16 normy;

      collision_intersects(&cur,col,&depth,&normx,&normy);
      if(depth>0)
      {
         //Compare collision resolution posiblilities
         RvR_fix16 depth_z;
         if(cur.z>col->z)
            depth_z = (col->z+col->col_height)-(cur.z);
         else
            depth_z = -((cur.z+cur.col_height)-col->z);

         if(RvR_abs(depth_z)>depth)
         {
            cur.x-=RvR_fix16_mul(normx,depth);
            cur.y-=RvR_fix16_mul(normy,depth);
            //cur.pos.x-=(normal.x*depth)/1024;
            //cur.pos.y-=(normal.y*depth)/1024;
         }
         else
         {
            cur.z+=depth_z;
            if(floor_height!=NULL&&depth_z>0&&col->z+col->col_height>*floor_height)
               *floor_height = col->z+col->col_height;
            else if(ceiling_height!=NULL&&depth_z<0&&col->z<*ceiling_height)
               *ceiling_height = col->z;
         }
      }

next:
      col = col->next;
   }

   //RvR_vec3 offset;
   RvR_fix16 offx = cur.x-e->x;
   RvR_fix16 offy = cur.y-e->y;
   RvR_fix16 offz = cur.z-e->z;
   RvR_fix16 oldx = cur.x;
   RvR_fix16 oldy = cur.y;
   RvR_fix16 oldz = cur.z;
   //RvR_vec3 old_pos = cur.pos;
   oldx-=offx;
   oldy-=offy;
   oldz-=offz;

   int16_t x_square_new, y_square_new;

   RvR_fix16 cornerx;
   RvR_fix16 cornery;
   RvR_fix16 corner_newx;
   RvR_fix16 corner_newy;
   //RvR_vec2 corner; // BBox corner in the movement direction
   //RvR_vec2 corner_new;

   int16_t x_dir = offx>0?1:-1;
   int16_t y_dir = offy>0?1:-1;

   cornerx = e->x+x_dir*CAMERA_COLL_RADIUS;
   cornery = e->y+y_dir*CAMERA_COLL_RADIUS;

   //int16_t x_square = RvR_div_round_down(corner.x,1024);
   //int16_t y_square = RvR_div_round_down(corner.y,1024);
   int16_t x_square = (cornerx/65536);
   int16_t y_square = (cornery/65536);

   corner_newx = cornerx+offx;
   corner_newy = cornery+offy;

   //x_square_new = RvR_div_round_down(corner_new.x,1024);
   //y_square_new = RvR_div_round_down(corner_new.y,1024);
   x_square_new = (corner_newx/65536);
   y_square_new = (corner_newy/65536);

   RvR_fix16 bottom_limit = -1024*65536;
   RvR_fix16 top_limit = 1024*65536;

   RvR_fix16 curr_ceil_height = 1024*65536;


   int can_step = 0;
   int did_step = 0;
   if(e->vz<=0)
      can_step = 1;

   bottom_limit = e->z;
   top_limit = e->z+e->col_height;

   curr_ceil_height = RvR_ray_map_ceiling_height_at(map_current(),x_square,y_square);

   // checks a single square for collision against the camera
#define collCheck(dir,s1,s2)\
   RvR_fix16 height = RvR_ray_map_floor_height_at(map_current(),s1,s2);\
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
      RvR_fix16 height2 = RvR_ray_map_ceiling_height_at(map_current(),s1,s2);\
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
      /*int16_t dir2##_square2 = RvR_div_round_down(corner.dir2-dir2##_dir */\
      /*CAMERA_COLL_RADIUS*2,1024);*/\
      int16_t dir2##_square2 = ((corner##dir2-dir2##_dir * CAMERA_COLL_RADIUS*2)/65536);\
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
            corner_new##dir = (dir##_square)*65536+\
            65536/2+dir##_dir*(65536/2)-\
            dir##_dir;\

         collHandle(x)
         collHandle(y)

      }
      else
      {
         /* Player collides without moving in the plane; this can happen e.g. on
         elevators due to vertical only movement. This code can get executed
         when force == 1. */

         //RvR_vec2 square_pos;
         //RvR_vec2 new_pos;

         RvR_fix16 square_posx = x_square*65536;
         RvR_fix16 square_posy = y_square*65536;

         RvR_fix16 new_posx = RvR_max(square_posx+CAMERA_COLL_RADIUS+1,RvR_min(square_posx+65536-CAMERA_COLL_RADIUS-1,e->x));
         RvR_fix16 new_posy = RvR_max(square_posy+CAMERA_COLL_RADIUS+1,RvR_min(square_posy+65536-CAMERA_COLL_RADIUS-1,e->y));

         corner_newx = cornerx+(new_posx-e->x);
         corner_newy = cornery+(new_posy-e->y);
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
            corner_newx = cornerx;
            corner_newy = cornery;
         }
      }
   }

   //RvR_vec3 new_pos;
   RvR_fix16 new_posx = corner_newx-x_dir*CAMERA_COLL_RADIUS;
   RvR_fix16 new_posy = corner_newy-y_dir*CAMERA_COLL_RADIUS;
   RvR_fix16 new_posz = e->z+offz;
   grid_entity_update_pos(e,new_posx,new_posy,new_posz);

   //int16_t x_square1 = RvR_div_round_down(e->pos.x-CAMERA_COLL_RADIUS,1024);
   int16_t x_square1 = ((e->x-CAMERA_COLL_RADIUS)/65536);

   //int16_t x_square2 = RvR_div_round_down(e->pos.x+CAMERA_COLL_RADIUS,1024);
   int16_t x_square2 = ((e->x+CAMERA_COLL_RADIUS)/65536);

   //int16_t y_square1 = RvR_div_round_down(e->pos.y-CAMERA_COLL_RADIUS,1024);
   int16_t y_square1 = ((e->y-CAMERA_COLL_RADIUS)/65536);

   //int16_t y_square2 = RvR_div_round_down(e->pos.y+CAMERA_COLL_RADIUS,1024);
   int16_t y_square2 = ((e->y+CAMERA_COLL_RADIUS)/65536);

   bottom_limit = RvR_ray_map_floor_height_at(map_current(),x_square1,y_square1);
   top_limit = RvR_ray_map_ceiling_height_at(map_current(),x_square1,y_square1);

   RvR_fix16 height;

#define checkSquares(s1,s2)\
   {\
      height = RvR_ray_map_floor_height_at(map_current(),x_square##s1,y_square##s2);\
      bottom_limit = RvR_max(bottom_limit,height);\
      height = RvR_ray_map_ceiling_height_at(map_current(),x_square##s1,y_square##s2);\
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

   if(e->z<bottom_limit&&did_step)
      e->vis_zoff = RvR_min(e->z-bottom_limit,e->vis_zoff);

   e->z = RvR_clamp(e->z,bottom_limit,top_limit-e->col_height);

#undef checkSquares
#undef collCheckOrtho
#undef collHandle
#undef collCheck

   if((e->vx)!=(e->x-oldx))
      e->vx = (e->x-oldx);
   if((e->vy)!=(e->y-oldy))
      e->vy = (e->y-oldy);

   //Lower velocity/friction
   //e->vx = (e->vel.x*900)/1024;
   //e->vy = (e->vel.y*900)/1024;
   e->vx = RvR_fix16_mul(e->vx,57600);
   e->vy = RvR_fix16_mul(e->vy,57600);
   //RvR_fix22 vel_mag = (e->vel.x*e->vel.x+e->vel.y*e->vel.y)/1024;
   RvR_fix16 vel_mag = RvR_fix16_mul(e->vx,e->vx)+RvR_fix16_mul(e->vy,e->vy);
   if(vel_mag<4)
      e->vx = e->vy = 0;
}

static void collision_intersects(Entity *a, Entity *b, RvR_fix16 *depth, RvR_fix16 *normalx, RvR_fix16 *normaly)
{
   *normalx = 0;
   *normaly = 0;
   *depth = 0;

   if(a->removed||b->removed)
      return;

   RvR_fix16 dx = b->x-a->x;
   RvR_fix16 dy = b->y-a->y;

   //Early out, overflow protection
   if(RvR_abs(dx)>a->col_radius+b->col_radius)
      return;
   if(RvR_abs(dy)>a->col_radius+b->col_radius)
      return;

   //RvR_fix16 depth2 = (dx*dx+dy*dy)/1024;
   RvR_fix16 depth2 = RvR_fix16_mul(dx,dx)+RvR_fix16_mul(dy,dy);
   RvR_fix16 r = a->col_radius+b->col_radius;
   if(depth2<r)
   {
      *depth = RvR_fix16_sqrt(depth2);
      if(*depth==0)
      {
         *normalx = 0;
         *normaly = 65536;
      }
      else
      {
         //normal->x = (dx*1024)/(*depth);
         //normal->y = (dy*1024)/(*depth);
         *normalx = RvR_fix16_div(dx,*depth);
         *normaly = RvR_fix16_div(dy,*depth);
      }
      *depth = r-*depth;
   }
}
//-------------------------------------
