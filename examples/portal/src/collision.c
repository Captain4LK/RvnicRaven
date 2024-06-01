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

#if 0
static void collision_movex(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy);
static void collision_movey(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy);
#endif
static void collision_project(RvR_fix22 vx, RvR_fix22 vy, RvR_fix22 bx, RvR_fix22 by, RvR_fix22 *ox, RvR_fix22 *oy);
static void collision_movexy(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy);

static RvR_fix22 collision_point_segment_dist2(const RvR_fix22 p[2], const RvR_fix22 a[2], const RvR_fix22 b[2], RvR_fix22 proj[2]);
static int collision_intersection_segment(const RvR_fix22 a[2], RvR_fix22 b[2], RvR_fix22 c[2], RvR_fix22 d[2], RvR_fix22 inter[2]);
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

   RvR_fix22 tot_mag = 0;
   for(int i = 0;i<2;i++)
   {
      RvR_fix22 ox = e->x;
      RvR_fix22 oy = e->y;

      //printf("%d: %d %d\n",i,e->vx,e->vy);
      nvx = e->vx;
      nvy = e->vy;
      collision_movexy(state,e,&nvx,&nvy);
      if(i==0)
      {
         vx = nvx;
         vy = nvy;
      }
      //printf("(%d %d) (%d %d)\n",nvx,nvy,e->vx,e->vy);
      //break;
      //collision_movey(state,e,&nvx,&nvy);

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
   //entity_update_pos(state,e,e->x,e->y+vy/48,e->z);

   e->vx = RvR_fix22_mul(e->vx,856);
   e->vy = RvR_fix22_mul(e->vy,856);
}

static void collision_movexy(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy)
{
   RvR_fix22 v[2];
   v[0] = e->vx;
   v[1] = e->vy;

   //NOTE(Captain4LK): all this 64 bit math is needed to stay accurate
   RvR_fix22 mag = RvR_fix22_sqrt64((int64_t)(e->vx)*(int64_t)(e->vx)+(int64_t)(e->vy)*(int64_t)(e->vy))/32;

   if(mag/48==0)
      return;

   RvR_fix22 np[2];
   np[0] = mag/48;
   np[1] = 0;
   RvR_fix22 nv[2];
   nv[0] = e->vx;
   nv[1] = e->vy;

   RvR_fix22 min_p[2];
   min_p[0] = np[0];
   min_p[1] = np[1];

   //puts("START");

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
         uint16_t wall_id = state->map->sectors[sector].wall_first+j;

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
         p0[0]-=e->x;
         p0[1]-=e->y;
         p1[0]-=e->x;
         p1[1]-=e->y;

         //Rotate --> (e->vx,e->vy) is positive x-axis
         RvR_fix22 tmp = p0[0];
         p0[0] = ((int64_t)p0[0]*v[0]+(int64_t)p0[1]*v[1])/(int64_t)RvR_non_zero(mag);
         p0[1] = ((int64_t)-tmp*v[1]+(int64_t)p0[1]*v[0])/(int64_t)RvR_non_zero(mag);
         tmp = p1[0];
         p1[0] = ((int64_t)p1[0]*v[0]+(int64_t)p1[1]*v[1])/(int64_t)RvR_non_zero(mag);
         p1[1] = ((int64_t)-tmp*v[1]+(int64_t)p1[1]*v[0])/(int64_t)RvR_non_zero(mag);

         //Back facing walls are non solid
         //Prevents clipping out of bounds by sliding
         //along backface of wall at corners
         if(p1[1]<=p0[1])
            continue;

         RvR_fix22 zero[2] = {0};
         RvR_fix22 cp[2] = {0};
         int line_inter = collision_intersection_segment(zero,np,p0,p1,cp);
         if(!line_inter)
         {
            RvR_fix22 proj0[2] = {0};
            RvR_fix22 proj1[2] = {0};
            RvR_fix22 proj2[2] = {0};
            RvR_fix22 proj3[2] = {0};
            RvR_fix22 dist_min = 0;
            RvR_fix22 dist = 0;
            int min_i = 0;

            dist_min = collision_point_segment_dist2(zero,p0,p1,proj0);

            dist = collision_point_segment_dist2(np,p0,p1,proj1);
            if(dist<dist_min)
            {
               min_i = 1;
               dist_min = dist;
            }

            dist = collision_point_segment_dist2(p0,zero,np,proj2);
            if(dist<dist_min)
            {
               min_i = 2;
               dist_min = dist;
            }

            dist = collision_point_segment_dist2(p1,zero,np,proj3);
            if(dist<dist_min)
            {
               min_i = 3;
               dist_min = dist;
            }

            if(dist_min>=RvR_fix22_mul(e->col_radius,e->col_radius))
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
            int found;
            for(int s = 0;s<RvR_array_length(collision_sector_stack);s++)
            {
               if(collision_sector_stack[s]==wall->portal)
               {
                  found = 1;
                  break;
               }
            }
            if(!found)
               RvR_array_push(collision_sector_stack,wall->portal);

            //If stepable --> skip
            //TODO(Captain4LK): slopes
            //TODO(Captain4LK): stepping
            RvR_fix22 floor = state->map->sectors[wall->portal].floor;
            RvR_fix22 ceiling = state->map->sectors[wall->portal].ceiling;
            if(e->z>=floor&&e->z+e->col_height<=ceiling)
               continue;
         }

         RvR_fix22 rp[2];
         RvR_fix22 proj[2];
         int64_t len2 = (int64_t)(p1[0]-p0[0])*(int64_t)(p1[0]-p0[0]);
         len2+=(int64_t)(p1[1]-p0[1])*(int64_t)(p1[1]-p0[1]);
         RvR_fix22 len = RvR_fix22_sqrt64(len2)/32;
         int64_t t = (int64_t)(cp[0]-p0[0])*(int64_t)(p1[0]-p0[0]);
         t+=(int64_t)(cp[1]-p0[1])*(int64_t)(p1[1]-p0[1]);
         proj[0] = (RvR_fix22)(p0[0]+(t*(p1[0]-p0[0]))/RvR_non_zero(len2));
         proj[1] = (RvR_fix22)(p0[1]+(t*(p1[1]-p0[1]))/RvR_non_zero(len2));
         int64_t dist2 = (int64_t)(proj[0]-cp[0])*(int64_t)(proj[0]-cp[0]);
         dist2+=(int64_t)(proj[1]-cp[1])*(int64_t)(proj[1]-cp[1]);
         RvR_fix22 dist = RvR_fix22_sqrt64(dist2)/32;
         //RvR_fix22 len2 = RvR_fix22_mul(p1[0]-p0[0],p1[0]-p0[0])+RvR_fix22_mul(p1[1]-p0[1],p1[1]-p0[1]);
         //RvR_fix22 rp[2];
         //RvR_fix22 proj[2];
         //RvR_fix22 t = RvR_fix22_mul(cp[0]-p0[0],p1[0]-p0[0])+RvR_fix22_mul(cp[1]-p0[1],p1[1]-p0[1]);
         //proj[0] = p0[0]+RvR_fix22_div(RvR_fix22_mul(t,p1[0]-p0[0]),RvR_non_zero(len2));
         //proj[1] = p0[1]+RvR_fix22_div(RvR_fix22_mul(t,p1[1]-p0[1]),RvR_non_zero(len2));
         //RvR_fix22 dist2 = RvR_fix22_mul(proj[0]-cp[0],proj[0]-cp[0])+RvR_fix22_mul(proj[1]-cp[1],proj[1]-cp[1]);

         {
            RvR_fix22 p10[2];
            p10[0] = p1[0]-p0[0];
            p10[1] = p1[1]-p0[1];
            //RvR_fix22 len = RvR_fix22_sqrt(len2);
            RvR_fix22 rp_dist = e->col_radius-dist;

            if(p0[1]<p1[1])
            {
               rp[0] = (RvR_fix22)(proj[0]+((int64_t)(p10[1]*rp_dist))/RvR_non_zero(len));
               rp[1] = (RvR_fix22)(proj[1]-((int64_t)(p10[0]*rp_dist))/RvR_non_zero(len));
               //rp[1] = (RvR_fix22)(proj[1]-RvR_fix22_div(RvR_fix22_mul(p10[0],rp_dist),RvR_non_zero(len)));
               //rp[0] = proj[0]+RvR_fix22_div(RvR_fix22_mul(p10[1],rp_dist),RvR_non_zero(len));
               //rp[1] = proj[1]-RvR_fix22_div(RvR_fix22_mul(p10[0],rp_dist),RvR_non_zero(len));
            }
            else
            {
               rp[0] = (RvR_fix22)(proj[0]-((int64_t)(p10[1]*rp_dist))/RvR_non_zero(len));
               rp[1] = (RvR_fix22)(proj[1]+((int64_t)(p10[0]*rp_dist))/RvR_non_zero(len));
               //rp[0] = proj[0]-RvR_fix22_div(RvR_fix22_mul(p10[1],rp_dist),RvR_non_zero(len));
               //rp[1] = proj[1]+RvR_fix22_div(RvR_fix22_mul(p10[0],rp_dist),RvR_non_zero(len));
            }
         }

         RvR_fix22 nx = 0;
         if((p1[1]-p0[1]>=0&&rp[1]-p0[1]<0)||
            (p1[1]-p0[1]<0&&rp[1]-p0[1]>0))
         {
            RvR_fix22 root = RvR_fix22_sqrt64(((int64_t)e->col_radius*e->col_radius)-(int64_t)(cp[1]-p0[1])*(int64_t)(cp[1]-p0[1]))/32;
            //RvR_fix22 root = RvR_fix22_sqrt(RvR_fix22_mul(e->col_radius,e->col_radius)-RvR_fix22_mul(cp[1]-p0[1],cp[1]-p0[1]));
            nx = p0[0]-root-1;
            printf("p0 %d (%d %d) (%d %d)\n",wall_id,p0[0],p0[1],p1[0],p1[1]);

            //TODO(Captain4LK): maybe only do this projection if the closest point was p0/p1, not the resolving point
            if(nx<=min_p[0])
               collision_project(e->vx,e->vy,-(p0_org[1]-(e->y+RvR_fix22_div(RvR_fix22_mul(e->vy,nx),RvR_non_zero(mag)))),
                     p0_org[0]-(e->x+RvR_fix22_div(RvR_fix22_mul(e->vx,nx),RvR_non_zero(mag))),&nv[0],&nv[1]);
   //entity_update_pos(state,e,e->x+RvR_fix22_div(RvR_fix22_mul(e->vx,min_p[0]),RvR_non_zero(mag)),e->y+RvR_fix22_div(RvR_fix22_mul(e->vy,min_p[0]),RvR_non_zero(mag)),e->z);
               //collision_project(e->vx,e->vy,-(p0[1]),p0[0]-nx,&nv[0],&nv[1]);
               //collision_project(e->vx,e->vy,p1_org[0]-p0_org[0],p1_org[1]-p0_org[1],&nv[0],&nv[1]);
         }
         else if((p1[1]-p0[1]>=0&&rp[1]-p0[1]>p1[1]-p0[1])||
                 (p1[1]-p0[1]<0&&rp[1]-p0[1]<p1[1]-p0[1]))
         {
            RvR_fix22 root = RvR_fix22_sqrt64((int64_t)e->col_radius*(int64_t)e->col_radius-(int64_t)(cp[1]-p1[1])*(int64_t)(cp[1]-p1[1]))/32;
            //RvR_fix22 root = RvR_fix22_sqrt(RvR_fix22_mul(e->col_radius,e->col_radius)-RvR_fix22_mul(cp[1]-p1[1],cp[1]-p1[1]));
            nx = p1[0]-root-1;
            printf("p1 %d (%d %d) (%d %d)\n",wall_id,p0[0],p0[1],p1[0],p1[1]);

            if(nx<=min_p[0])
               collision_project(e->vx,e->vy,-(p1_org[1]-(e->y+RvR_fix22_div(RvR_fix22_mul(e->vy,nx),RvR_non_zero(mag)))),
                     p1_org[0]-(e->x+RvR_fix22_div(RvR_fix22_mul(e->vx,nx),RvR_non_zero(mag))),&nv[0],&nv[1]);
               //collision_project(e->vx,e->vy,p1_org[0]-p0_org[0],p1_org[1]-p0_org[1],&nv[0],&nv[1]);
         }
         else
         {
            RvR_fix22 x_resolv = (RvR_fix22)(p0[0]+((int64_t)(p1[0]-p0[0])*(int64_t)(rp[1]-p0[1]))/RvR_non_zero(p1[1]-p0[1]));
            //RvR_fix22 x_resolv = p0[0]+RvR_fix22_div(RvR_fix22_mul(p1[0]-p0[0],rp[1]-p0[1]),RvR_non_zero(p1[1]-p0[1]));
            nx = cp[0]+(x_resolv-rp[0])-1;
            printf("mid %d (%d %d) (%d %d)\n",wall_id,p0[0],p0[1],p1[0],p1[1]);

            if(nx<=min_p[0])
               collision_project(e->vx,e->vy,p1_org[0]-p0_org[0],p1_org[1]-p0_org[1],&nv[0],&nv[1]);
         }

         min_p[0] = RvR_min(min_p[0],nx);

         printf("dist %d\n",collision_point_segment_dist2(min_p,p0,p1,proj));
      }
   }

   //if(min_p[0]>=0)
   {
      *vx = nv[0];
      *vy = nv[1];
   }
   min_p[0] = RvR_max(min_p[0],0);
   entity_update_pos(state,e,e->x+RvR_fix22_div(RvR_fix22_mul(e->vx,min_p[0]),RvR_non_zero(mag)),e->y+RvR_fix22_div(RvR_fix22_mul(e->vy,min_p[0]),RvR_non_zero(mag)),e->z);
}

#if 0
static void collision_movexy(Gamestate *state, Entity *e, RvR_fix22 *vx, RvR_fix22 *vy)
{
   RvR_fix22 v[2];
   v[0] = e->vx;
   v[1] = e->vy;
   //v[0] = e->vx/48;
   //v[1] = e->vy/48;
   //NOTE(Captain4LK): all this 64 bit math is needed to stay accurate
   //RvR_fix22 mag2 = RvR_fix22_mul(e->vx/48,e->vx/48)+RvR_fix22_mul(e->vy/48,e->vy/48);
   //RvR_fix22 mag2 = RvR_fix22_mul(e->vx/48,e->vx/48)+RvR_fix22_mul(e->vy/48,e->vy/48);
   RvR_fix22 mag = RvR_fix22_sqrt((int64_t)(e->vx)*(int64_t)(e->vx)+(int64_t)(e->vy)*(int64_t)(e->vy))/32;
   //RvR_fix22 mag = RvR_fix22_sqrt((int64_t)(e->vx/48)*(int64_t)(e->vx/48)+(int64_t)(e->vy/48)*(int64_t)(e->vy/48))/32;
   //printf("%d\n",mag);
   //RvR_fix22 mag = RvR_fix22_sqrt(mag2);

   //if(mag/48==0)
      //return;

   RvR_fix22 np[2];
   np[0] = mag/48;
   np[1] = 0;
   RvR_fix22 nv[2];
   nv[0] = e->vx;
   nv[1] = e->vy;

   RvR_fix22 min_p[2];
   min_p[0] = np[0];
   min_p[1] = np[1];

   //puts("START");

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
         p0[0]-=e->x;
         p0[1]-=e->y;
         p1[0]-=e->x;
         p1[1]-=e->y;

         //Rotate --> (e->vx,e->vy) is positive x-axis
         RvR_fix22 tmp = p0[0];
         p0[0] = ((int64_t)p0[0]*v[0]+(int64_t)p0[1]*v[1])/(int64_t)RvR_non_zero(mag);
         p0[1] = ((int64_t)-tmp*v[1]+(int64_t)p0[1]*v[0])/(int64_t)RvR_non_zero(mag);
         tmp = p1[0];
         p1[0] = ((int64_t)p1[0]*v[0]+(int64_t)p1[1]*v[1])/(int64_t)RvR_non_zero(mag);
         p1[1] = ((int64_t)-tmp*v[1]+(int64_t)p1[1]*v[0])/(int64_t)RvR_non_zero(mag);

         //Back facing walls are non solid
         //Prevents clipping out of bounds by sliding along backface of wall
         if(p1[1]<p0[1])
            continue;

         RvR_fix22 len2 = RvR_fix22_mul(p1[0]-p0[0],p1[0]-p0[0])+RvR_fix22_mul(p1[1]-p0[1],p1[1]-p0[1]);
         RvR_fix22 t = RvR_fix22_mul(np[0]-p0[0],p1[0]-p0[0])+RvR_fix22_mul(np[1]-p0[1],p1[1]-p0[1]);
         RvR_fix22 t_clipped = RvR_max(0,RvR_min(len2,t));

         RvR_fix22 proj[2];
         proj[0] = p0[0]+RvR_fix22_div(RvR_fix22_mul(t_clipped,p1[0]-p0[0]),RvR_non_zero(len2));
         proj[1] = p0[1]+RvR_fix22_div(RvR_fix22_mul(t_clipped,p1[1]-p0[1]),RvR_non_zero(len2));
         RvR_fix22 dist2 = RvR_fix22_mul(proj[0]-np[0],proj[0]-np[0])+RvR_fix22_mul(proj[1]-np[1],proj[1]-np[1]);
         if(dist2>RvR_fix22_mul(e->col_radius,e->col_radius))
            continue;


         //Intersection
         if(wall->portal!=RVR_PORT_SECTOR_INVALID)
         {
            //Check if already added
            int found;
            for(int s = 0;s<RvR_array_length(collision_sector_stack);s++)
            {
               if(collision_sector_stack[s]==wall->portal)
               {
                  found = 1;
                  break;
               }
            }
            if(!found)
               RvR_array_push(collision_sector_stack,wall->portal);

            //If stepable --> skip
            //TODO(Captain4LK): slopes
            //TODO(Captain4LK): stepping
            RvR_fix22 floor = state->map->sectors[wall->portal].floor;
            RvR_fix22 ceiling = state->map->sectors[wall->portal].ceiling;
            if(e->z>=floor&&e->z+e->col_height<=ceiling)
               continue;
         }

         RvR_fix22 rp[2];
         proj[0] = p0[0]+RvR_fix22_div(RvR_fix22_mul(t,p1[0]-p0[0]),RvR_non_zero(len2));
         proj[1] = p0[1]+RvR_fix22_div(RvR_fix22_mul(t,p1[1]-p0[1]),RvR_non_zero(len2));
         dist2 = RvR_fix22_mul(proj[0]-np[0],proj[0]-np[0])+RvR_fix22_mul(proj[1]-np[1],proj[1]-np[1]);

         {
            RvR_fix22 p10[2];
            p10[0] = p1[0]-p0[0];
            p10[1] = p1[1]-p0[1];
            RvR_fix22 len = RvR_fix22_sqrt(len2);
            RvR_fix22 rp_dist = e->col_radius-RvR_fix22_sqrt(dist2);

            if(p0[1]<p1[1])
            {
               rp[0] = proj[0]+RvR_fix22_div(RvR_fix22_mul(p10[1],rp_dist),RvR_non_zero(len));
               rp[1] = proj[1]-RvR_fix22_div(RvR_fix22_mul(p10[0],rp_dist),RvR_non_zero(len));
            }
            else
            {
               rp[0] = proj[0]-RvR_fix22_div(RvR_fix22_mul(p10[1],rp_dist),RvR_non_zero(len));
               rp[1] = proj[1]+RvR_fix22_div(RvR_fix22_mul(p10[0],rp_dist),RvR_non_zero(len));
            }
         }

         RvR_fix22 nx = 0;
         if((p1[1]-p0[1]>=0&&rp[1]-p0[1]<0)||
            (p1[1]-p0[1]<0&&rp[1]-p0[1]>=0))
         {
            RvR_fix22 root = RvR_fix22_sqrt(RvR_fix22_mul(e->col_radius,e->col_radius)-RvR_fix22_mul(np[1]-p0[1],np[1]-p0[1]));
            nx = p0[0]-root-1;
         }
         else if((p1[1]-p0[1]>=0&&rp[1]-p0[1]>p1[1]-p0[1])||
                 (p1[1]-p0[1]<0&&rp[1]-p0[1]<p1[1]-p0[1]))
         {
            RvR_fix22 root = RvR_fix22_sqrt(RvR_fix22_mul(e->col_radius,e->col_radius)-RvR_fix22_mul(np[1]-p1[1],np[1]-p1[1]));
            nx = p1[0]-root-1;
         }
         else
         {
            RvR_fix22 x_resolv = p0[0]+RvR_fix22_div(RvR_fix22_mul(p1[0]-p0[0],rp[1]-p0[1]),RvR_non_zero(p1[1]-p0[1]));
            nx = np[0]+(x_resolv-rp[0])-1;
         }

         if(nx<=min_p[0])
            collision_project(e->vx,e->vy,p1_org[0]-p0_org[0],p1_org[1]-p0_org[1],&nv[0],&nv[1]);
         min_p[0] = RvR_min(min_p[0],nx);

         //proj[0] = p0[0]+RvR_fix22_div(RvR_fix22_mul(t_clipped,p1[0]-p0[0]),RvR_non_zero(len2));
         //proj[1] = p0[1]+RvR_fix22_div(RvR_fix22_mul(t_clipped,p1[1]-p0[1]),RvR_non_zero(len2));
         //dist2 = RvR_fix22_mul(proj[0]-min_p[0],proj[0]-min_p[0])+RvR_fix22_mul(proj[1]-min_p[1],proj[1]-min_p[1]);
         //printf("after %d\n",RvR_fix22_sqrt(dist2));
      }
   }

   //printf("min %d\n",min_p[0]);
   //if(min_p[0]>=0)
   {
      *vx = nv[0];
      *vy = nv[1];
   }
   min_p[0] = RvR_max(min_p[0],0);
   entity_update_pos(state,e,e->x+RvR_fix22_div(RvR_fix22_mul(e->vx,min_p[0]),RvR_non_zero(mag)),e->y+RvR_fix22_div(RvR_fix22_mul(e->vy,min_p[0]),RvR_non_zero(mag)),e->z);
}
#endif

static void collision_project(RvR_fix22 vx, RvR_fix22 vy, RvR_fix22 bx, RvR_fix22 by, RvR_fix22 *ox, RvR_fix22 *oy)
{
   RvR_fix22 b2 = RvR_fix22_mul(bx,bx)+RvR_fix22_mul(by,by);
   RvR_fix22 ab = RvR_fix22_mul(vx,bx)+RvR_fix22_mul(vy,by);

   *ox = RvR_fix22_div(RvR_fix22_mul(bx,ab),RvR_non_zero(b2));
   *oy = RvR_fix22_div(RvR_fix22_mul(by,ab),RvR_non_zero(b2));
   //RvR_fix22 factor = 
}

static RvR_fix22 collision_point_segment_dist2(const RvR_fix22 p[2], const RvR_fix22 a[2], const RvR_fix22 b[2], RvR_fix22 proj[2])
{
   int64_t len2 = (int64_t)(b[0]-a[0])*(int64_t)(b[0]-a[0]);
   len2+=(int64_t)(b[1]-a[1])*(int64_t)(b[1]-a[1]);
   int64_t t = (int64_t)(p[0]-a[0])*(int64_t)(b[0]-a[0]);
   t+=(int64_t)(p[1]-a[1])*(int64_t)(b[1]-a[1]);
   t = RvR_max(0,RvR_min(len2,t));

   proj[0] = (RvR_fix22)(a[0]+(t*(b[0]-a[0]))/RvR_non_zero(len2));
   proj[1] = (RvR_fix22)(a[1]+(t*(b[1]-a[1]))/RvR_non_zero(len2));

   int64_t dist2 = (int64_t)(proj[0]-p[0])*(int64_t)(proj[0]-p[0]);
   dist2+=(int64_t)(proj[1]-p[1])*(int64_t)(proj[1]-p[1]);

   return (RvR_fix22)(dist2/1024);

   //RvR_fix22 len2 = RvR_fix22_mul(b[0]-a[0],b[0]-a[0])+RvR_fix22_mul(b[1]-a[1],b[1]-a[1]);
   //RvR_fix22 t = RvR_fix22_mul(p[0]-a[0],b[0]-a[0])+RvR_fix22_mul(p[1]-a[1],b[1]-a[1]);
   //t = RvR_max(0,RvR_min(len2,t));

   //proj[0] = a[0]+RvR_fix22_div(RvR_fix22_mul(t,b[0]-a[0]),RvR_non_zero(len2));
   //proj[1] = a[1]+RvR_fix22_div(RvR_fix22_mul(t,b[1]-a[1]),RvR_non_zero(len2));
   //RvR_fix22 dist2 = RvR_fix22_mul(proj[0]-p[0],proj[0]-p[0])+RvR_fix22_mul(proj[1]-p[1],proj[1]-p[1]);

   //return dist2;
}

//This function makes assumption for this specific use case
//a[0]>b[0], a[1] = b[1] = 0
static int collision_intersection_segment(const RvR_fix22 a[2], RvR_fix22 b[2], RvR_fix22 c[2], RvR_fix22 d[2], RvR_fix22 inter[2])
{
   int64_t denom = (int64_t)(d[1]-c[1])*(int64_t)(b[0]-a[0])-(int64_t)(d[0]-c[0])*(int64_t)(b[1]-a[1]);
   int64_t num0 = (int64_t)(d[0]-c[0])*(int64_t)(a[1]-c[1])-(int64_t)(d[1]-c[1])*(int64_t)(a[0]-c[0]);
   int64_t num1 = (int64_t)(b[0]-a[0])*(int64_t)(a[1]-c[1])-(int64_t)(b[1]-a[1])*(int64_t)(a[0]-c[0]);

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
         
         inter[0] = RvR_max(a[0],c[0]);
         inter[1] = 0;
         return 1;
      }
      else
      {
         if(d[0]>b[0])
            return 0;
         if(c[0]<a[0])
            return 0;

         inter[0] = RvR_max(a[0],d[0]);
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

   inter[0] = a[0]+((b[0]-a[0])*num0)/denom;
   inter[1] = a[1]+((b[1]-a[1])*num0)/denom;

   return 1;
}
//-------------------------------------
