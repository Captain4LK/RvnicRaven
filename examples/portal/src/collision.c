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
static void collision_project(RvR_fix22 vx, RvR_fix22 vy, RvR_fix22 bx, RvR_fix22 by, RvR_fix22 *ox, RvR_fix22 *oy);
static void collision_movexy(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy);
static void collision_movez(Gamestate *state, Entity *e);

static RvR_fix22 collision_point_segment_dist2(const RvR_fix22 p[2], const RvR_fix22 a[2], const RvR_fix22 b[2], RvR_fix22 proj[2]);
static int collision_intersection_segment(const RvR_fix22 a[2], RvR_fix22 b[2], RvR_fix22 c[2], RvR_fix22 d[2], RvR_fix22 inter[2]);
//-------------------------------------

//Function implementations

void collision_move(Gamestate *state, Entity *e)
{
   if(e==NULL||e->removed)
      return;

   RvR_fix22 vx = e->vel[0];
   RvR_fix22 vy = e->vel[1];
   RvR_fix22 nvx = vx;
   RvR_fix22 nvy = vy;
   RvR_fix22 mag_v = RvR_fix22_sqrt64((int64_t)vx*vx+(int64_t)vy*vy)/32;

   RvR_fix22 tot_mag = 0;
   for(int i = 0; i<2; i++)
   {
      RvR_fix22 ox = e->pos[0];
      RvR_fix22 oy = e->pos[1];

      nvx = e->vel[0];
      nvy = e->vel[1];
      collision_movexy(state, e, &nvx, &nvy);
      if(i==0)
      {
         vx = nvx;
         vy = nvy;
      }

      RvR_fix22 dx = e->pos[0] - ox;
      RvR_fix22 dy = e->pos[1] - oy;
      tot_mag += RvR_fix22_sqrt64((int64_t)dx*dx+(int64_t)dy*dy)/32;
      if(tot_mag*48>=mag_v)
         break;

      RvR_fix22 mag_n = RvR_fix22_sqrt64((int64_t)nvx*nvx+(int64_t)nvy*nvy)/32;
      if(mag_n==0)
         break;
      if(mag_n + tot_mag*48>mag_v)
      {
         nvx = (nvx*(mag_v-tot_mag*48))/RvR_non_zero(mag_n);
         nvy = (nvy*(mag_v-tot_mag*48))/RvR_non_zero(mag_n);
      }

      e->vel[0] = nvx;
      e->vel[1] = nvy;
   }

   if(!RvR_key_down(RVR_KEY_P))
      collision_movez(state, e);
   e->vel[0] = vx;
   e->vel[1] = vy;

   e->vel[0] = RvR_fix22_mul(e->vel[0], 856);
   e->vel[1] = RvR_fix22_mul(e->vel[1], 856);
}

static void collision_movexy(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy)
{
   RvR_fix22 v[2];
   v[0] = e->vel[0];
   v[1] = e->vel[1];

   //NOTE(Captain4LK): all this 64 bit math is needed to stay accurate
   RvR_fix22 mag = RvR_fix22_sqrt64((int64_t)(e->vel[0]) * (int64_t)(e->vel[0]) + (int64_t)(e->vel[1]) * (int64_t)(e->vel[1])) / 32;

   //TODO(Captain4LK): if we want to do the pushing out thing, we can't skip collision in this case
   if(mag / 48==0)
      return;

   RvR_fix22 np[2];
   np[0] = mag / 48;
   np[1] = 0;
   RvR_fix22 nv[2];
   nv[0] = e->vel[0];
   nv[1] = e->vel[1];

   RvR_fix22 min_p[2];
   min_p[0] = np[0];
   min_p[1] = np[1];

   //TODO(Captain4LK): don't remove from stack, so that we can check if already added
   //should be faster than using up to 8KB bitmap
   RvR_array_length_set(collision_sector_stack, 0);
   if(!RvR_key_down(RVR_KEY_C))
      RvR_array_push(collision_sector_stack, e->sector);
   for(int i = 0; i<RvR_array_length(collision_sector_stack); i++)
   {
      uint16_t sector = collision_sector_stack[i];

      for(int j = 0; j<state->map->sectors[sector].wall_count; j++)
      {
         RvR_port_wall *wall = state->map->walls + state->map->sectors[sector].wall_first + j;

         //TODO(Captain4LK): aabb for quick reject?

         RvR_fix22 p0[2];
         RvR_fix22 p1[2];
         RvR_fix22 p0_org[2];
         RvR_fix22 p1_org[2];
         p0_org[0] = p0[0] = wall->x;
         p0_org[1] = p0[1] = wall->y;
         p1_org[0] = p1[0] = state->map->walls[wall->p2].x;
         p1_org[1] = p1[1] = state->map->walls[wall->p2].y;

         //Translate --> (e->x,e->y) is origin
         p0[0] -= e->pos[0];
         p0[1] -= e->pos[1];
         p1[0] -= e->pos[0];
         p1[1] -= e->pos[1];

         //Rotate --> (e->vx,e->vy) is positive x-axis
         RvR_fix22 tmp = p0[0];
         p0[0] = (RvR_fix22)(((int64_t)p0[0] * v[0] + (int64_t)p0[1] * v[1]) / (int64_t)RvR_non_zero(mag));
         p0[1] = (RvR_fix22)(((int64_t)-tmp * v[1] + (int64_t)p0[1] * v[0]) / (int64_t)RvR_non_zero(mag));
         tmp = p1[0];
         p1[0] = (RvR_fix22)(((int64_t)p1[0] * v[0] + (int64_t)p1[1] * v[1]) / (int64_t)RvR_non_zero(mag));
         p1[1] = (RvR_fix22)(((int64_t)-tmp * v[1] + (int64_t)p1[1] * v[0]) / (int64_t)RvR_non_zero(mag));

         //Back facing walls are non solid
         //Prevents clipping out of bounds by sliding
         //along backface of wall at corners
         if(p1[1]<=p0[1])
            continue;

         RvR_fix22 zero[2] = {0};
         RvR_fix22 cp[2] = {0};
         int line_inter = collision_intersection_segment(zero, np, p0, p1, cp);
         if(!line_inter)
         {
            RvR_fix22 proj0[2] = {0};
            RvR_fix22 proj1[2] = {0};
            RvR_fix22 proj2[2] = {0};
            RvR_fix22 proj3[2] = {0};
            RvR_fix22 dist_min = 0;
            RvR_fix22 dist = 0;
            int min_i = 0;

            dist_min = collision_point_segment_dist2(zero, p0, p1, proj0);

            dist = collision_point_segment_dist2(np, p0, p1, proj1);
            if(dist<dist_min)
            {
               min_i = 1;
               dist_min = dist;
            }

            dist = collision_point_segment_dist2(p0, zero, np, proj2);
            if(dist<dist_min)
            {
               min_i = 2;
               dist_min = dist;
            }

            dist = collision_point_segment_dist2(p1, zero, np, proj3);
            if(dist<dist_min)
            {
               min_i = 3;
               dist_min = dist;
            }

            if(dist_min>=RvR_fix22_mul(e->radius, e->radius))
               continue;

            if(min_i==0)
            {
               cp[0] = 0;
               cp[1] = 0;
            }
            else if(min_i==1)
            {
               cp[0] = np[0];
               cp[1] = np[1];
            }
            else if(min_i==2)
            {
               cp[0] = proj2[0];
               cp[1] = proj2[1];
            }
            else if(min_i==3)
            {
               cp[0] = proj3[0];
               cp[1] = proj3[1];
            }
         }

         //Intersection
         if(wall->portal!=RVR_PORT_SECTOR_INVALID)
         {
            //Check if already added
            int found = 0;
            for(int s = 0; s<RvR_array_length(collision_sector_stack); s++)
            {
               if(collision_sector_stack[s]==wall->portal)
               {
                  found = 1;
                  break;
               }
            }
            if(!found)
               RvR_array_push(collision_sector_stack, wall->portal);

            //If stepable --> skip
            //We need to real coordinates of the collision point (cp) here,
            //so we need to inverse the transformation from before
            RvR_fix22 cp_inv[2];
            cp_inv[0] = (RvR_fix22)(((int64_t)cp[0] * v[0] - (int64_t)cp[1] * v[1]) / RvR_non_zero(mag) + e->pos[0]);
            cp_inv[1] = (RvR_fix22)(((int64_t)cp[0] * v[1] + (int64_t)cp[1] * v[0]) / RvR_non_zero(mag) + e->pos[1]);
            RvR_fix22 floor = RvR_port_sector_floor_at(state->map, wall->portal, cp_inv[0], cp_inv[1]);
            RvR_fix22 ceiling = RvR_port_sector_ceiling_at(state->map, wall->portal, cp_inv[0], cp_inv[1]);

            if(e->pos[2] + e->step_height>=floor&&e->pos[2] + e->height<=ceiling&&ceiling - floor>=e->height)
            {
               continue;
            }
         }

         RvR_fix22 rp[2];
         RvR_fix22 proj[2];
         int64_t len2 = (int64_t)(p1[0] - p0[0]) * (int64_t)(p1[0] - p0[0]);
         len2 += (int64_t)(p1[1] - p0[1]) * (int64_t)(p1[1] - p0[1]);
         RvR_fix22 len = RvR_fix22_sqrt64(len2) / 32;
         int64_t t = (int64_t)(cp[0] - p0[0]) * (int64_t)(p1[0] - p0[0]);
         t += (int64_t)(cp[1] - p0[1]) * (int64_t)(p1[1] - p0[1]);
         proj[0] = (RvR_fix22)(p0[0] + (t * (p1[0] - p0[0])) / RvR_non_zero(len2));
         proj[1] = (RvR_fix22)(p0[1] + (t * (p1[1] - p0[1])) / RvR_non_zero(len2));
         int64_t dist2 = (int64_t)(proj[0] - cp[0]) * (int64_t)(proj[0] - cp[0]);
         dist2 += (int64_t)(proj[1] - cp[1]) * (int64_t)(proj[1] - cp[1]);
         RvR_fix22 dist = RvR_fix22_sqrt64(dist2) / 32;

         {
            RvR_fix22 p10[2];
            p10[0] = p1[0] - p0[0];
            p10[1] = p1[1] - p0[1];
            RvR_fix22 rp_dist = e->radius - dist;

            if(p0[1]<p1[1])
            {
               rp[0] = (RvR_fix22)(proj[0] + ((int64_t)(p10[1] * rp_dist)) / RvR_non_zero(len));
               rp[1] = (RvR_fix22)(proj[1] - ((int64_t)(p10[0] * rp_dist)) / RvR_non_zero(len));
            }
            else
            {
               rp[0] = (RvR_fix22)(proj[0] - ((int64_t)(p10[1] * rp_dist)) / RvR_non_zero(len));
               rp[1] = (RvR_fix22)(proj[1] + ((int64_t)(p10[0] * rp_dist)) / RvR_non_zero(len));
            }
         }

         RvR_fix22 nx = 0;
         if((p1[1] - p0[1]>=0&&rp[1] - p0[1]<0)||
            (p1[1] - p0[1]<0&&rp[1] - p0[1]>0))
         {
            RvR_fix22 root = RvR_fix22_sqrt64(((int64_t)e->radius * e->radius) - (int64_t)(cp[1] - p0[1]) * (int64_t)(cp[1] - p0[1])) / 32;
            nx = p0[0] - root - 2;

            //TODO(Captain4LK): maybe only do this projection if the closest point was p0/p1, not the resolving point
            if(nx<=min_p[0])
               collision_project(e->vel[0], e->vel[1], (RvR_fix22)(-(p0_org[1] - (e->pos[1] + ((int64_t)e->vel[1]*nx)/RvR_non_zero(mag)))),
                                 (RvR_fix22)(p0_org[0] - (e->pos[0] + ((int64_t)e->vel[0]*nx)/RvR_non_zero(mag))), &nv[0], &nv[1]);
         }
         else if((p1[1] - p0[1]>=0&&rp[1] - p0[1]>p1[1] - p0[1])||
                 (p1[1] - p0[1]<0&&rp[1] - p0[1]<p1[1] - p0[1]))
         {
            RvR_fix22 root = RvR_fix22_sqrt64((int64_t)e->radius * (int64_t)e->radius - (int64_t)(cp[1] - p1[1]) * (int64_t)(cp[1] - p1[1])) / 32;
            nx = p1[0] - root - 2;

            if(nx<=min_p[0])
               collision_project(e->vel[0], e->vel[1], (RvR_fix22)(-(p1_org[1] - (e->pos[1] + ((int64_t)e->vel[1]*nx)/RvR_non_zero(mag)))),
                                 (RvR_fix22)(p1_org[0] - (e->pos[0] + ((int64_t)e->vel[0]*nx)/RvR_non_zero(mag))), &nv[0], &nv[1]);
         }
         else
         {
            RvR_fix22 x_resolv = (RvR_fix22)(p0[0] + ((int64_t)(p1[0] - p0[0]) * (int64_t)(rp[1] - p0[1])) / RvR_non_zero(p1[1] - p0[1]));
            nx = cp[0] + (x_resolv - rp[0]) - 2;

            if(nx<=min_p[0])
               collision_project(e->vel[0], e->vel[1], p1_org[0] - p0_org[0], p1_org[1] - p0_org[1], &nv[0], &nv[1]);
         }

         min_p[0] = RvR_max(0, RvR_min(min_p[0], nx));
      }
   }

   *vx = nv[0];
   *vy = nv[1];

   //TODO(Captain4LK): give velocity to push out in this case?
   //Could automatically handle moving/rotating sectors
   //TODO(Captain4LK): Currently we get negative values even on static geometry, look into this
   if(min_p[0]<0)
   {
      //printf("%d\n",min_p[0]);
   }
   min_p[0] = RvR_max(min_p[0], 0);

   entity_update_pos(state, e,
                     (RvR_fix22)(e->pos[0] + ((int64_t)e->vel[0] * min_p[0]) / RvR_non_zero(mag)),
                     (RvR_fix22)(e->pos[1] + ((int64_t)e->vel[1] * min_p[0]) / RvR_non_zero(mag)),
                     e->pos[2]);
}

static void collision_movez(Gamestate *state, Entity *e)
{
   RvR_fix22 p[2];
   p[0] = e->pos[0];
   p[1] = e->pos[1];

   RvR_fix22 floor = RvR_port_sector_floor_at(state->map, e->sector, e->pos[0], e->pos[1]);
   RvR_fix22 ceiling = RvR_port_sector_ceiling_at(state->map, e->sector, e->pos[0], e->pos[1]);

   //TODO(Captain4LK): don't remove from stack, so that we can check if already added
   //should be faster than using up to 8KB bitmap
   RvR_array_length_set(collision_sector_stack, 0);
   RvR_array_push(collision_sector_stack, e->sector);
   for(int i = 0; i<RvR_array_length(collision_sector_stack); i++)
   {
      uint16_t sector = collision_sector_stack[i];

      for(int j = 0; j<state->map->sectors[sector].wall_count; j++)
      {
         RvR_port_wall *wall = state->map->walls + state->map->sectors[sector].wall_first + j;

         RvR_fix22 p0[2];
         RvR_fix22 p1[2];
         p0[0] = wall->x;
         p0[1] = wall->y;
         p1[0] = state->map->walls[wall->p2].x;
         p1[1] = state->map->walls[wall->p2].y;

         RvR_fix22 proj[2];
         RvR_fix22 dist2 = collision_point_segment_dist2(p, p0, p1, proj);
         if(dist2>=RvR_fix22_mul(e->radius, e->radius))
            continue;

         if(wall->portal!=RVR_PORT_SECTOR_INVALID)
         {
            //Check if already added
            int found = 0;
            for(int s = 0; s<RvR_array_length(collision_sector_stack); s++)
            {
               if(collision_sector_stack[s]==wall->portal)
               {
                  found = 1;
                  break;
               }
            }
            if(!found)
               RvR_array_push(collision_sector_stack, wall->portal);

            //TODO(Captain4LK): sometimes we still end up inside of walls here, so we only allow steppable surfaces here
            RvR_fix22 sfloor = RvR_port_sector_floor_at(state->map, wall->portal, proj[0], proj[1]);
            RvR_fix22 sceiling = RvR_port_sector_ceiling_at(state->map, wall->portal, proj[0], proj[1]);
            if(e->pos[2] + e->step_height>=sfloor&&e->pos[2] + e->height<=sceiling&&sceiling - sfloor>=e->height)
            {
               floor = RvR_max(floor, sfloor);
               ceiling = RvR_min(ceiling, sceiling);
            }
         }
      }
   }

   e->pos[2] = RvR_min(ceiling - e->height, RvR_max(e->pos[2] + e->vel[2] / 64, floor));

   if(e->pos[2] + e->height>=ceiling)
   {
      e->vel[2] = 0;
   }

   if(e->pos[2]==floor)
   {
      e->on_ground = 1;
      e->vel[2] = 0;
   }
   else
   {
      e->on_ground = 0;
   }
}

static void collision_project(RvR_fix22 vx, RvR_fix22 vy, RvR_fix22 bx, RvR_fix22 by, RvR_fix22 *ox, RvR_fix22 *oy)
{
   int64_t b2 = (int64_t)bx*bx+(int64_t)by*by;
   int64_t ab = (int64_t)vx*bx+(int64_t)vy*by;

   *ox = (RvR_fix22)((bx*ab)/RvR_non_zero(b2));
   *oy = (RvR_fix22)((by*ab)/RvR_non_zero(b2));
}

static RvR_fix22 collision_point_segment_dist2(const RvR_fix22 p[2], const RvR_fix22 a[2], const RvR_fix22 b[2], RvR_fix22 proj[2])
{
   int64_t len2 = (int64_t)(b[0] - a[0]) * (int64_t)(b[0] - a[0]);
   len2 += (int64_t)(b[1] - a[1]) * (int64_t)(b[1] - a[1]);
   int64_t t = (int64_t)(p[0] - a[0]) * (int64_t)(b[0] - a[0]);
   t += (int64_t)(p[1] - a[1]) * (int64_t)(b[1] - a[1]);
   t = RvR_max(0, RvR_min(len2, t));

   proj[0] = (RvR_fix22)(a[0] + (t * (b[0] - a[0])) / RvR_non_zero(len2));
   proj[1] = (RvR_fix22)(a[1] + (t * (b[1] - a[1])) / RvR_non_zero(len2));

   int64_t dist2 = (int64_t)(proj[0] - p[0]) * (int64_t)(proj[0] - p[0]);
   dist2 += (int64_t)(proj[1] - p[1]) * (int64_t)(proj[1] - p[1]);

   return (RvR_fix22)(dist2 / 1024);
}

//This function makes assumption for this specific use case
//a[0]>b[0], a[1] = b[1] = 0
static int collision_intersection_segment(const RvR_fix22 a[2], RvR_fix22 b[2], RvR_fix22 c[2], RvR_fix22 d[2], RvR_fix22 inter[2])
{
   int64_t denom = (int64_t)(d[1] - c[1]) * (int64_t)(b[0] - a[0]) - (int64_t)(d[0] - c[0]) * (int64_t)(b[1] - a[1]);
   int64_t num0 = (int64_t)(d[0] - c[0]) * (int64_t)(a[1] - c[1]) - (int64_t)(d[1] - c[1]) * (int64_t)(a[0] - c[0]);
   int64_t num1 = (int64_t)(b[0] - a[0]) * (int64_t)(a[1] - c[1]) - (int64_t)(b[1] - a[1]) * (int64_t)(a[0] - c[0]);

   //TODO(Captain4LK): we skip lines where y0 == y1, do we need this section at all?
   //Lines identical
   if(denom==0&&num0==0&&num1==0)
   {
      //For this use case, we want this to be the result
      if(c[0]<d[0])
      {
         if(c[0]>b[0])
            return 0;
         if(d[0]<a[0])
            return 0;

         inter[0] = RvR_max(a[0], c[0]);
         inter[1] = 0;
         return 1;
      }
      else
      {
         if(d[0]>b[0])
            return 0;
         if(c[0]<a[0])
            return 0;

         inter[0] = RvR_max(a[0], d[0]);
         inter[1] = 0;
         return 1;
      }
   }

   //Lines paralell
   if(denom==0)
      return 0;

   //No intersection
   if(denom>0)
   {
      if(num0>denom)
         return 0;
      if(num0<0)
         return 0;
      if(num1>denom)
         return 0;
      if(num1<0)
         return 0;
   }
   else
   {
      if(num0<denom)
         return 0;
      if(num0>0)
         return 0;
      if(num1<denom)
         return 0;
      if(num1>0)
         return 0;
   }

   inter[0] = (RvR_fix22)(a[0] + ((b[0] - a[0]) * num0) / denom);
   inter[1] = (RvR_fix22)(a[1] + ((b[1] - a[1]) * num0) / denom);

   return 1;
}
//-------------------------------------
