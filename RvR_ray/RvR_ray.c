/*
RvnicRaven - raycast

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR_ray_config.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_ray_cast_multi_hit(const RvR_ray_map *map, RvR_ray r, RvR_ray_hit_result *hits, int *hit_count, int max_steps)
{
   *hit_count = 0;

   RvR_fix16 current_posx = r.x;
   RvR_fix16 current_posy = r.y;
   RvR_fix16 current_squarex = r.x/65536;
   RvR_fix16 current_squarey = r.y/65536;

   RvR_fix16 deltax = RvR_abs(RvR_fix16_div(65536,RvR_non_zero(r.dirx)));
   RvR_fix16 deltay = RvR_abs(RvR_fix16_div(65536,RvR_non_zero(r.diry)));

   RvR_fix16 stepx;
   RvR_fix16 stepy;
   RvR_fix16 side_distx;
   RvR_fix16 side_disty;
   int side = 0;

   if(r.dirx<0)
   {
      stepx = -1;
      side_distx = RvR_fix16_mul(r.x-current_squarex*65536,deltax);
   }
   else
   {
      stepx = 1;
      side_distx = RvR_fix16_mul(current_squarex*65536+65536-r.x,deltax);
   }

   if(r.diry<0)
   {
      stepy = -1;
      side_disty = RvR_fix16_mul(r.y-current_squarey*65536,deltay);
   }
   else
   {
      stepy = 1;
      side_disty = RvR_fix16_mul(current_squarey*65536+65536-r.y,deltay);
   }

   for(unsigned i = 0;i<max_steps;i++)
   {
      // DDA step
      if(side_distx<side_disty)
      {
         side_distx+=deltax;
         current_squarex+=stepx;
         side = 0;
      }
      else
      {
         side_disty+=deltay;
         current_squarey+=stepy;
         side = 1;
      }

      RvR_ray_hit_result h;
      h.posx = current_posx;
      h.posy = current_posy;
      h.squarex = current_squarex;
      h.squarey = current_squarey;

      if(!side)
      {
         h.distance = (side_distx-deltax);
         h.posy = r.y+RvR_fix16_mul(h.distance,r.diry);
         h.posx = current_squarex*65536;
         h.direction = 3;
         if(stepx==-1)
         {
            h.direction = 1;
            h.posx+=65536;
         }
      }
      else
      {
         h.distance = (side_disty-deltay);
         h.posx = r.x+RvR_fix16_mul(h.distance,r.dirx);
         h.posy = current_squarey*65536;
         h.direction = 2;
         if(stepy==-1)
         {
            h.direction = 0;
            h.posy+=65536;
         }
      }
      
      if(RvR_ray_map_inbounds(map,current_squarex,current_squarey))
      {
         h.wall_ftex = RvR_ray_map_wall_ftex_at_us(map,current_squarex,current_squarey);
         h.wall_ctex = RvR_ray_map_wall_ctex_at_us(map,current_squarex,current_squarey);
      }
      else
      {
         h.wall_ftex = map->sky_tex;
         h.wall_ctex = map->sky_tex;
      }

      h.fheight = 0;
      h.cheight = (127*65536)/8;
      h.floor_tex = map->sky_tex;
      h.ceil_tex = map->sky_tex;

      switch(h.direction)
      {
      case 0:
         h.texture_coord = (h.posx)&65535;
         if(RvR_ray_map_inbounds(map,current_squarex,current_squarey+1))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex,current_squarey+1);
            h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex,current_squarey+1);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex,h.squarey+1);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex,h.squarey+1);
         }
         break;
      case 1:
         h.texture_coord = (-h.posy)&65535;
         if(RvR_ray_map_inbounds(map,current_squarex+1,current_squarey))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex+1,current_squarey);
            h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex+1,current_squarey);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex+1,h.squarey);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex+1,h.squarey);
         }
         break;
      case 2:
         h.texture_coord = (-h.posx)&65535;
         if(RvR_ray_map_inbounds(map,current_squarex,current_squarey-1))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex,current_squarey-1);
            h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex,current_squarey-1);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex,h.squarey-1);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex,h.squarey-1);
         }
         break;
      case 3:
         h.texture_coord = (h.posy)&65535;
         if(RvR_ray_map_inbounds(map,current_squarex-1,current_squarey))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex-1,current_squarey);
            h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex-1,current_squarey);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex-1,h.squarey);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex-1,h.squarey);
         }
         break;
      default:
         h.texture_coord = 0;
         break;
      }

      hits[*hit_count] = h;
      (*hit_count)++;

      if(*hit_count>=max_steps)
         break;
   }
}
//-------------------------------------
