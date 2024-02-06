/*
RvnicRaven retro game engine

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "undo.h"
#include "map.h"
#include "editor.h"
#include "color.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
RvR_port_wall *sd_walls = NULL;
int16_t sd_wall_overlap;

int sd_split;
int16_t sd_split_sector;
int16_t sd_split_wall;
int16_t sd_split_wall_end;
//-------------------------------------

//Function prototypes
static int sector_draw_split();
static int sector_draw_connect();
//-------------------------------------

//Function implementations

void sector_draw_start(RvR_fix22 x, RvR_fix22 y)
{
   RvR_array_length_set(sd_walls,0);
   sd_wall_overlap = -1;

   sd_split = 0;
   sd_split_wall = -1;
   sd_split_sector = -1;
   sd_split_wall_end = -1;

   RvR_port_wall w = {0};
   w.x = x;
   w.y = y;
   w.p2 = 1;
   w.portal = -1;
   w.portal_wall = -1;
   RvR_array_push(sd_walls,w);

   //Search for overlapping walls
   for(int i = 0;i<map->wall_count;i++)
   {
      RvR_port_wall *p0 = map->walls+i;
      if(p0->x==x&&p0->y==y)
         sd_wall_overlap = i;
   }
}

int sector_draw_add(RvR_fix22 x, RvR_fix22 y)
{
   //If not back at first wall, add new wall at (x,y)
   if(x!=sd_walls[0].x||y!=sd_walls[0].y)
   {
      int overlap = 0;
      for(int i = 0;i<RvR_array_length(sd_walls);i++)
      {
         if(sd_walls[i].x==x&&sd_walls[i].y==y)
         {
            overlap = 1;
            break;
         }
      }

      if(!overlap)
      {
         //Check for split on second wall
         if(RvR_array_length(sd_walls)==1)
         {
            RvR_fix22 avg_x = sd_walls[0].x+x;
            RvR_fix22 avg_y = sd_walls[0].y+y;
            avg_x/=2;
            avg_y/=2;

            for(int i = 0;i<map->sector_count;i++)
            {
               //Middle of line in sector
               if(!RvR_port_sector_inside(map,i,avg_x,avg_y))
                  continue;

               //Check for overlapping wall
               int16_t wall_overlap = -1;
               for(int j = 0;j<map->sectors[i].wall_count;j++)
               {
                  RvR_port_wall *w = map->walls+map->sectors[i].wall_first+j;
                  if(w->x==sd_walls[0].x&&w->y==sd_walls[0].y)
                  {
                     wall_overlap = map->sectors[i].wall_first+j;
                     break;
                  }
               }

               //Not adjacent to first point
               if(wall_overlap>=0)
               {
                  int16_t prev = RvR_port_wall_previous(map,wall_overlap);
                  int16_t next = RvR_port_wall_next(map,wall_overlap);
                  if((map->walls[prev].x!=x||map->walls[prev].y!=y)&&
                     (map->walls[next].x!=x||map->walls[next].y!=y))
                  {
                     sd_split = 1;
                     sd_split_sector = i;
                     sd_split_wall = wall_overlap;
                     break;
                  }
               }
            }
         }

         //Add new wall
         RvR_port_wall w = {0};
         w.x = x;
         w.y = y;
         w.p2 = RvR_array_length(sd_walls)+1;
         w.portal = -1;
         w.portal_wall = -1;
         RvR_array_push(sd_walls,w);

         //Search for overlapping walls
         for(int i = 0;i<map->wall_count;i++)
         {
            RvR_port_wall *p0 = map->walls+i;
            if(p0->x==x&&p0->y==y)
               sd_wall_overlap = i;
         }
      }
   }

   //Complete loop
   if(RvR_array_length(sd_walls)>2&&!sd_split&&x==sd_walls[0].x&&y==sd_walls[0].y)
   {
      sd_walls[RvR_array_length(sd_walls)-1].p2 = 0;

      //No connections
      if(sd_wall_overlap==-1)
      {
         //Check if inside other sector
         int16_t sector_inside = -1;
         for(int i = 0;i<map->sector_count;i++)
         {
            sector_inside = i;
            for(int j = 0;j<RvR_array_length(sd_walls);j++)
            {
               if(!RvR_port_sector_inside(map,i,sd_walls[j].x,sd_walls[j].y))
               {
                  sector_inside = -1;
                  break;
               }
            }

            if(sector_inside>=0)
               break;
         }

         //Standalone sector
         if(sector_inside==-1)
         {
            //Add new sector
            int16_t sector = map->sector_count++;
            map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
            map->sectors[sector].wall_count = RvR_array_length(sd_walls);
            map->sectors[sector].wall_first = map->wall_count;
            map->sectors[sector].floor = 0;
            map->sectors[sector].ceiling = 2*1024;
            map->sectors[sector].floor_tex = 15;
            map->sectors[sector].ceiling_tex = 15;
            map->sectors[sector].x_off = 0;
            map->sectors[sector].y_off = 0;
            map->sectors[sector].flags = 0;
            map->wall_count+=RvR_array_length(sd_walls);
            map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
            for(int i = 0;i<RvR_array_length(sd_walls);i++)
            {
               map->walls[map->sectors[sector].wall_first+i].x = sd_walls[i].x;
               map->walls[map->sectors[sector].wall_first+i].y = sd_walls[i].y;
               map->walls[map->sectors[sector].wall_first+i].p2 = sd_walls[i].p2+map->sectors[sector].wall_first;
               map->walls[map->sectors[sector].wall_first+i].flags = sd_walls[i].flags;
               map->walls[map->sectors[sector].wall_first+i].shade_offset = sd_walls[i].shade_offset;
               map->walls[map->sectors[sector].wall_first+i].x_off = sd_walls[i].x_off;
               map->walls[map->sectors[sector].wall_first+i].y_off = sd_walls[i].y_off;
               map->walls[map->sectors[sector].wall_first+i].tex_lower = sd_walls[i].tex_lower;
               map->walls[map->sectors[sector].wall_first+i].tex_upper = sd_walls[i].tex_upper;
               map->walls[map->sectors[sector].wall_first+i].tex_mid = sd_walls[i].tex_mid;
               map->walls[map->sectors[sector].wall_first+i].portal = -1;
               map->walls[map->sectors[sector].wall_first+i].portal_wall = -1;
               if(i==RvR_array_length(sd_walls)-1)
                  map->walls[map->sectors[sector].wall_first+i].p2 = map->sectors[sector].wall_first;
            }

            RvR_port_sector_fix_winding(map,sector);

            return 1;
         }
         //Inner sector
         else
         {
            //Add to existing sector
            map->wall_count+=RvR_array_length(sd_walls);
            map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
            int count = RvR_array_length(sd_walls);
            int16_t insert = map->sectors[sector_inside].wall_first+map->sectors[sector_inside].wall_count;

            //Move existing walls to right
            for(int w = map->wall_count-1;w>insert;w--)
               map->walls[w] = map->walls[w-count];

            //Fix indices
            for(int i = 0;i<map->wall_count;i++)
            {
               RvR_port_wall *wall = map->walls+i;
               if(wall->portal>=insert)
                  wall->portal+=count;
               if(wall->p2>=insert)
                  wall->p2+=count;
               if(wall->portal_wall>=insert)
                  wall->portal_wall+=count;
            }
            //Update sectors first wall
            for(int i = 0;i<map->sector_count;i++)
            {
               RvR_port_sector *sect = map->sectors+i;
               if(sect->wall_first>=insert)
                  sect->wall_first+=count;
            }

            //Insert walls
            for(int i = 0;i<RvR_array_length(sd_walls);i++)
            {
               RvR_port_wall *wall = map->walls+(map->sectors[sector_inside].wall_count+map->sectors[sector_inside].wall_first+i);
               wall->x = sd_walls[i].x;
               wall->y = sd_walls[i].y;
               wall->p2 = sd_walls[i].p2+map->sectors[sector_inside].wall_first+map->sectors[sector_inside].wall_count;
               wall->flags = sd_walls[i].flags;
               wall->shade_offset = sd_walls[i].shade_offset;
               wall->x_off = sd_walls[i].x_off;
               wall->y_off = sd_walls[i].y_off;
               wall->tex_lower = sd_walls[i].tex_lower;
               wall->tex_upper = sd_walls[i].tex_upper;
               wall->tex_mid = sd_walls[i].tex_mid;
               wall->portal = -1;
               wall->portal_wall = -1;
               if(i==RvR_array_length(sd_walls)-1)
                  wall->p2 = map->sectors[sector_inside].wall_first+map->sectors[sector_inside].wall_count;
            }
            map->sectors[sector_inside].wall_count+=count;
            RvR_port_sector_fix_winding(map,sector_inside);

            return 1;
         }
      }
      //Connected to other sector(s)
      else
      {
         //Add new sector
         int16_t sector = map->sector_count++;
         map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
         map->sectors[sector].wall_count = RvR_array_length(sd_walls);
         map->sectors[sector].wall_first = map->wall_count;
         map->sectors[sector].floor = 0;
         map->sectors[sector].ceiling = 2*1024;
         map->sectors[sector].floor_tex = 15;
         map->sectors[sector].ceiling_tex = 15;
         map->wall_count+=RvR_array_length(sd_walls);
         map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
         for(int i = 0;i<RvR_array_length(sd_walls);i++)
         {
            map->walls[map->sectors[sector].wall_first+i].x = sd_walls[i].x;
            map->walls[map->sectors[sector].wall_first+i].y = sd_walls[i].y;
            map->walls[map->sectors[sector].wall_first+i].p2 = sd_walls[i].p2+map->sectors[sector].wall_first;
            map->walls[map->sectors[sector].wall_first+i].flags = sd_walls[i].flags;
            map->walls[map->sectors[sector].wall_first+i].shade_offset = sd_walls[i].shade_offset;
            map->walls[map->sectors[sector].wall_first+i].x_off = sd_walls[i].x_off;
            map->walls[map->sectors[sector].wall_first+i].y_off = sd_walls[i].y_off;
            map->walls[map->sectors[sector].wall_first+i].tex_lower = sd_walls[i].tex_lower;
            map->walls[map->sectors[sector].wall_first+i].tex_upper = sd_walls[i].tex_upper;
            map->walls[map->sectors[sector].wall_first+i].tex_mid = sd_walls[i].tex_mid;
            map->walls[map->sectors[sector].wall_first+i].portal = -1;
            map->walls[map->sectors[sector].wall_first+i].portal_wall = -1;
            if(i==RvR_array_length(sd_walls)-1)
               map->walls[map->sectors[sector].wall_first+i].p2 = map->sectors[sector].wall_first;
         }

         RvR_port_sector_fix_winding(map,sector);

         //Add portals to overlaping sectors
         for(int i = 0;i<map->sectors[sector].wall_count;i++)
         {
            int16_t swall = map->sectors[sector].wall_first+i;
            int16_t snext = RvR_port_wall_next(map,swall);
            int16_t sprev = RvR_port_wall_previous(map,swall);

            for(int j = 0;j<map->wall_count-map->sectors[sector].wall_count;j++)
            {
               int16_t next = RvR_port_wall_next(map,j);
               int16_t prev = RvR_port_wall_previous(map,j);
               int16_t wall_sector = RvR_port_wall_sector(map,j);

               if(map->walls[j].x==map->walls[swall].x&&map->walls[j].y==map->walls[swall].y)
               {
                  if(map->walls[next].x==map->walls[sprev].x&&map->walls[next].y==map->walls[sprev].y)
                  {
                     map->walls[sprev].portal = wall_sector;
                     map->walls[j].portal = sector;
                     map->walls[sprev].portal_wall = j;
                     map->walls[j].portal_wall = sprev;
                  }
               }
            }
         }

         return 1;
      }
   }
   //Split sector
   else if(sd_split)
   {
      //Search for wall overlapping last point in sector to be split
      int16_t split_wall = -1;
      for(int i = 0;i<map->sectors[sd_split_sector].wall_count;i++)
      {
         RvR_port_wall *sw = map->walls+map->sectors[sd_split_sector].wall_first+i;
         RvR_port_wall *last_wall = sd_walls+RvR_array_length(sd_walls)-1;
         if(sw->x==last_wall->x&&sw->y==last_wall->y)
         {
            split_wall = map->sectors[sd_split_sector].wall_first+i;
            break;
         }
      }

      //Not found or back at first wall
      if(split_wall==-1||split_wall==sd_split_wall)
         return 0;

      sd_split_wall_end = split_wall;

      //Check for overlapping walls of same sector loop
      for(int i = map->sectors[sd_split_sector].wall_first;i<map->sectors[sd_split_sector].wall_first+map->sectors[sd_split_sector].wall_count;i++)
      {
         if(i==sd_split_wall_end||i==sd_split_wall)
            continue;
         RvR_port_wall *w = map->walls+i;
         RvR_port_wall *ws = map->walls+sd_split_wall;
         RvR_port_wall *we = map->walls+sd_split_wall_end;
         if(w->x==ws->x&&w->y==ws->y)
         {
            RvR_log("Error: ambiguous start wall, two walls overlap\n");
            return 1;
         }
         else if(w->x==we->x&&w->y==we->y)
         {
            RvR_log("Error: ambiguous end wall, two walls overlap\n");
            return 1;
         }
      }

      //Check if split_wall and split_wall_end in same subsector
      int sub = 0;
      int can_split = 0;
      int found_start = 0;
      int found_end = 0;
      //printf("%d %d\n",sd_split_wall,sd_split_wall_end);
      for(int i = 0;i<map->sectors[sd_split_sector].wall_count;i++)
      {
         int iwall = map->sectors[sd_split_sector].wall_first+i;

         if(iwall==sd_split_wall)
            found_start = 1;
         if(iwall==sd_split_wall_end)
            found_end = 1;

         if(map->walls[iwall].p2<iwall)
         {
            if(found_start^found_end)
               break;
            if(found_start&&found_end)
            {
               can_split = 1;
               break;
            }
            sub++;
         }
      }

      if(can_split)
         return sector_draw_split();
      else
         return sector_draw_connect();
   }

   return 0;
}

int sector_draw_back()
{
   if(RvR_array_length(sd_walls)==1)
      return 1;

   if(RvR_array_length(sd_walls)==2)
   {
      sd_split = 0;
      sd_split_sector = -1;
      sd_split_wall = -1;
      sd_split_wall_end = -1;
   }

   RvR_array_length_set(sd_walls,RvR_array_length(sd_walls)-1);

   //Recheck overlapping walls
   sd_wall_overlap = -1;
   for(int i = 0;i<RvR_array_length(sd_walls);i++)
   {
      for(int j = 0;j<map->wall_count;j++)
      {
         RvR_port_wall *p0 = map->walls+j;
         if(p0->x==sd_walls[i].x&&p0->y==sd_walls[i].y)
            sd_wall_overlap = j;
      }
   }

   return 0;
}

void sector_draw_draw(RvR_fix22 x, RvR_fix22 y, RvR_fix22 zoom)
{
   RvR_port_wall w = {0};
   w.p2 = -1;
   w.x = x;
   w.y = y;
   RvR_array_push(sd_walls,w);

   for(int i = 0;i<RvR_array_length(sd_walls);i++)
   {
      RvR_port_wall *p0 = sd_walls+i;
      RvR_port_wall *p1 = sd_walls+p0->p2;

      if(p0->p2==-1||p0->p2>=RvR_array_length(sd_walls))
         continue;

      int x0 = ((p0->x-camera.x)*256)/RvR_non_zero(zoom)+RvR_xres()*128+128;
      int y0 = ((p0->y-camera.y)*256)/RvR_non_zero(zoom)+RvR_yres()*128+128;
      int x1 = ((p1->x-camera.x)*256)/RvR_non_zero(zoom)+RvR_xres()*128+128;
      int y1 = ((p1->y-camera.y)*256)/RvR_non_zero(zoom)+RvR_yres()*128+128;

      if(p0->portal>=0)
      {
         //Only draw one wall for portals
         if(p0->portal>i)
            RvR_render_line(x0,y0,x1,y1,color_red);
      }
      else
      {
         RvR_render_line(x0,y0,x1,y1,color_white);
      }
   }

   for(int i = 0;i<RvR_array_length(sd_walls);i++)
   {
      RvR_port_wall *p0 = sd_walls+i;

      int x0 = ((p0->x-camera.x))/RvR_non_zero(zoom)+RvR_xres()/2;
      int y0 = ((p0->y-camera.y))/RvR_non_zero(zoom)+RvR_yres()/2;

      RvR_render_rectangle(x0-2,y0-2,5,5,color_orange);
   }

   RvR_array_length_set(sd_walls,RvR_array_length(sd_walls)-1);
}

static int sector_draw_split()
{
   //Remove last wall, will be added again
   RvR_array_length_set(sd_walls,RvR_array_length(sd_walls)-1);
   int old_length = RvR_array_length(sd_walls);
   int split_length = old_length;

   int start = sd_split_wall_end;
   int end = sd_split_wall;
   int cur = start;
   do
   {
      RvR_port_wall w = map->walls[cur];
      w.p2 = RvR_array_length(sd_walls)+1;
      if(map->walls[cur].p2==end)
         w.p2 = 0;
      RvR_array_push(sd_walls,w);

      cur = map->walls[cur].p2;
   }
   while(cur!=end);

   int sub_split = RvR_port_wall_subsector(map,sd_split_sector,sd_split_wall);
   int last_copied = sub_split;
   int split0_start = 0;

   RvR_port_sector sec = {0};
   sec.wall_first = 0;
   sec.wall_count = RvR_array_length(sd_walls);
   RvR_port_map mp = {0};
   mp.sector_count = 1;
   mp.wall_count = RvR_array_length(sd_walls);
   mp.sectors = &sec;
   mp.walls = RvR_malloc(sizeof(*mp.walls)*mp.wall_count,"tmp wall array");
   memcpy(mp.walls,sd_walls,sizeof(*mp.walls)*mp.wall_count);

   int subsector = 0;
   for(int i = map->sectors[sd_split_sector].wall_first;i<map->sectors[sd_split_sector].wall_first+map->sectors[sd_split_sector].wall_count;i++)
   {
      if(subsector!=sub_split&&subsector!=last_copied)
      {
         last_copied = subsector;

         int old_len = RvR_array_length(sd_walls);

         //Check if first wall of subsector in copied sector
         int inside = RvR_port_sector_inside(&mp,0,map->walls[i].x,map->walls[i].y);
         printf("winding %d inside %d\n",RvR_port_wall_winding(&mp,0),inside);
         if(inside!=RvR_port_wall_winding(&mp,0))
         {
            int start = i;
            int end = i;
            int cur = start;
            int p2 = RvR_array_length(sd_walls);
            do
            {
               RvR_port_wall w = map->walls[cur];
               w.p2 = RvR_array_length(sd_walls)+1;
               if(map->walls[cur].p2==end)
                  w.p2 = p2;
               RvR_array_push(sd_walls,w);

               cur = map->walls[cur].p2;
            }
            while(cur!=start);

            if(subsector==0)
            {
               int rot_amount = RvR_array_length(sd_walls)-old_len;
               int rot_start = RvR_array_length(sd_walls)-rot_amount;
               split0_start+=rot_amount;

               //Reverse all
               for(int j = 0;j<RvR_array_length(sd_walls)/2;j++)
               {
                  RvR_port_wall tmp = sd_walls[j];
                  sd_walls[j] = sd_walls[RvR_array_length(sd_walls)-j-1];
                  sd_walls[RvR_array_length(sd_walls)-j-1] = tmp;
               }

               //Reverse last rot_start
               for(int j = 0;j<rot_start/2;j++)
               {
                  RvR_port_wall tmp = sd_walls[rot_amount+j];
                  sd_walls[rot_amount+j] = sd_walls[RvR_array_length(sd_walls)-j-1];
                  sd_walls[RvR_array_length(sd_walls)-j-1] = tmp;
               }

               //Reverse until rot_amount
               for(int j = 0;j<rot_amount/2;j++)
               {
                  RvR_port_wall tmp = sd_walls[j];
                  sd_walls[j] = sd_walls[rot_amount-j-1];
                  sd_walls[rot_amount-j-1] = tmp;
               }

               //Adjust indices
               for(int j = 0;j<RvR_array_length(sd_walls);j++)
                  sd_walls[j].p2 = (sd_walls[j].p2+rot_amount)%RvR_array_length(sd_walls);

               //printf("rot0 %d\n",rot_amount);
            }
         }
      }

      if(map->walls[i].p2<i)
         subsector++;
   }

   //Create sector from sd_walls
   int16_t sector0 = map->sector_count++;
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
   map->sectors[sector0].wall_count = RvR_array_length(sd_walls);
   map->sectors[sector0].wall_first = map->wall_count;
   map->sectors[sector0].floor = 0;
   map->sectors[sector0].ceiling = 2*1024;
   map->sectors[sector0].floor_tex = 15;
   map->sectors[sector0].ceiling_tex = 15;
   map->wall_count+=RvR_array_length(sd_walls);
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
   for(int i = 0;i<RvR_array_length(sd_walls);i++)
   {
      map->walls[map->sectors[sector0].wall_first+i].x = sd_walls[i].x;
      map->walls[map->sectors[sector0].wall_first+i].y = sd_walls[i].y;
      map->walls[map->sectors[sector0].wall_first+i].p2 = sd_walls[i].p2+map->sectors[sector0].wall_first;
      map->walls[map->sectors[sector0].wall_first+i].flags = sd_walls[i].flags;
      map->walls[map->sectors[sector0].wall_first+i].shade_offset = sd_walls[i].shade_offset;
      map->walls[map->sectors[sector0].wall_first+i].x_off = sd_walls[i].x_off;
      map->walls[map->sectors[sector0].wall_first+i].y_off = sd_walls[i].y_off;
      map->walls[map->sectors[sector0].wall_first+i].tex_lower = sd_walls[i].tex_lower;
      map->walls[map->sectors[sector0].wall_first+i].tex_upper = sd_walls[i].tex_upper;
      map->walls[map->sectors[sector0].wall_first+i].tex_mid = sd_walls[i].tex_mid;
      map->walls[map->sectors[sector0].wall_first+i].portal = sd_walls[i].portal;
      map->walls[map->sectors[sector0].wall_first+i].portal_wall = -1;
   }

   //Reset sd_walls
   if(split0_start>0)
   {
      for(int i = 0;i<old_length+1;i++)
      {
         sd_walls[i] = sd_walls[i+split0_start];
         sd_walls[i].p2 = i+1;
         sd_walls[i].portal = -1;
         sd_walls[i].portal_wall = -1;
      }
   }
   RvR_array_length_set(sd_walls,old_length+1);

   //Reverse first walls
   for(int i = 0;i<RvR_array_length(sd_walls)/2;i++)
   {
      RvR_port_wall tmp = sd_walls[i];
      sd_walls[i] = sd_walls[RvR_array_length(sd_walls)-1-i];
      sd_walls[RvR_array_length(sd_walls)-1-i] = tmp;
      sd_walls[i].p2 = i+1;
      sd_walls[RvR_array_length(sd_walls)-1-i].p2 = RvR_array_length(sd_walls)-i;
   }
   RvR_array_length_set(sd_walls,old_length);

   start = sd_split_wall;
   end = sd_split_wall_end;
   cur = start;
   do
   {
      RvR_port_wall w = map->walls[cur];
      w.p2 = RvR_array_length(sd_walls)+1;
      if(map->walls[cur].p2==end)
         w.p2 = 0;
      RvR_array_push(sd_walls,w);

      cur = map->walls[cur].p2;
   }
   while(cur!=end);

   sub_split = RvR_port_wall_subsector(map,sd_split_sector,sd_split_wall);
   last_copied = sub_split;

   subsector = 0;
   int split1_start = 0;
   for(int i = map->sectors[sd_split_sector].wall_first;i<map->sectors[sd_split_sector].wall_first+map->sectors[sd_split_sector].wall_count;i++)
   {
      if(subsector!=sub_split&&subsector!=last_copied)
      {
         last_copied = subsector;
         int old_len = RvR_array_length(sd_walls);

         //Check if first wall of subsector in copied sector
         int inside = RvR_port_sector_inside(&mp,0,map->walls[i].x,map->walls[i].y);
         //printf("winding %d inside %d\n",RvR_port_wall_winding(&mp,0),inside);
         if(inside==RvR_port_wall_winding(&mp,0))
         {
            int p2 = RvR_array_length(sd_walls);
            int start = i;
            int end = i;
            int cur = start;
            do
            {
               RvR_port_wall w = map->walls[cur];
               w.p2 = RvR_array_length(sd_walls)+1;
               if(map->walls[cur].p2==end)
                  w.p2 = p2;
               RvR_array_push(sd_walls,w);

               cur = map->walls[cur].p2;
            }
            while(cur!=end);
            
            if(subsector==0)
            {
               int rot_amount = RvR_array_length(sd_walls)-old_len;
               int rot_start = RvR_array_length(sd_walls)-rot_amount;
               split1_start+=rot_amount;

               //Reverse all
               for(int j = 0;j<RvR_array_length(sd_walls)/2;j++)
               {
                  RvR_port_wall tmp = sd_walls[j];
                  sd_walls[j] = sd_walls[RvR_array_length(sd_walls)-j-1];
                  sd_walls[RvR_array_length(sd_walls)-j-1] = tmp;
               }

               //Reverse last rot_start
               for(int j = 0;j<rot_start/2;j++)
               {
                  RvR_port_wall tmp = sd_walls[rot_amount+j];
                  sd_walls[rot_amount+j] = sd_walls[RvR_array_length(sd_walls)-j-1];
                  sd_walls[RvR_array_length(sd_walls)-j-1] = tmp;
               }

               //Reverse until rot_amount
               for(int j = 0;j<rot_amount/2;j++)
               {
                  RvR_port_wall tmp = sd_walls[j];
                  sd_walls[j] = sd_walls[rot_amount-j-1];
                  sd_walls[rot_amount-j-1] = tmp;
               }

               //Adjust indices
               for(int j = 0;j<RvR_array_length(sd_walls);j++)
                  sd_walls[j].p2 = (sd_walls[j].p2+rot_amount)%RvR_array_length(sd_walls);

               //printf("rot %d\n",rot_amount);
            }
         }
      }

      if(map->walls[i].p2<i)
         subsector++;
   }

   RvR_free(mp.walls);
   mp.walls = NULL;

   //Add new sector
   int16_t sector1 = map->sector_count++;
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
   map->sectors[sector1].wall_count = RvR_array_length(sd_walls);
   map->sectors[sector1].wall_first = map->wall_count;
   map->sectors[sector1].floor = 0;
   map->sectors[sector1].ceiling = 2*1024;
   map->sectors[sector1].floor_tex = 15;
   map->sectors[sector1].ceiling_tex = 15;
   map->wall_count+=RvR_array_length(sd_walls);
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
   for(int i = 0;i<RvR_array_length(sd_walls);i++)
   {
      map->walls[map->sectors[sector1].wall_first+i].x = sd_walls[i].x;
      map->walls[map->sectors[sector1].wall_first+i].y = sd_walls[i].y;
      map->walls[map->sectors[sector1].wall_first+i].p2 = sd_walls[i].p2+map->sectors[sector1].wall_first;
      map->walls[map->sectors[sector1].wall_first+i].flags = sd_walls[i].flags;
      map->walls[map->sectors[sector1].wall_first+i].shade_offset = sd_walls[i].shade_offset;
      map->walls[map->sectors[sector1].wall_first+i].x_off = sd_walls[i].x_off;
      map->walls[map->sectors[sector1].wall_first+i].y_off = sd_walls[i].y_off;
      map->walls[map->sectors[sector1].wall_first+i].tex_lower = sd_walls[i].tex_lower;
      map->walls[map->sectors[sector1].wall_first+i].tex_upper = sd_walls[i].tex_upper;
      map->walls[map->sectors[sector1].wall_first+i].tex_mid = sd_walls[i].tex_mid;
      map->walls[map->sectors[sector1].wall_first+i].portal = sd_walls[i].portal;
      map->walls[map->sectors[sector1].wall_first+i].portal_wall = -1;
   }

   //Fix sector windings
   RvR_port_sector_fix_winding(map,sector0);
   RvR_port_sector_fix_winding(map,sector1);

   //Fix links and portals
   for(int i = 0;i<map->sectors[sector0].wall_count;i++)
   {
      int16_t swall = map->sectors[sector0].wall_first+i;
      int16_t snext = RvR_port_wall_next(map,swall);
      int16_t sprev = RvR_port_wall_previous(map,swall);

      for(int j = 0;j<map->wall_count;j++)
      {
         int16_t next = RvR_port_wall_next(map,j);
         int16_t prev = RvR_port_wall_previous(map,j);
         int16_t wall_sector = RvR_port_wall_sector(map,j);

         if(wall_sector==sector0||wall_sector==sd_split_sector)
            continue;

         if(map->walls[j].x==map->walls[swall].x&&map->walls[j].y==map->walls[swall].y)
         {
            if(map->walls[next].x==map->walls[sprev].x&&map->walls[next].y==map->walls[sprev].y)
            {
               map->walls[sprev].portal = wall_sector;
               map->walls[j].portal = sector0;
               map->walls[sprev].portal_wall = j;
               map->walls[j].portal_wall = sprev;
            }
         }
      }
   }

   for(int i = 0;i<map->sectors[sector1].wall_count;i++)
   {
      int16_t swall = map->sectors[sector1].wall_first+i;
      int16_t snext = RvR_port_wall_next(map,swall);
      int16_t sprev = RvR_port_wall_previous(map,swall);

      for(int j = 0;j<map->wall_count;j++)
      {
         int16_t next = RvR_port_wall_next(map,j);
         int16_t prev = RvR_port_wall_previous(map,j);
         int16_t wall_sector = RvR_port_wall_sector(map,j);

         if(wall_sector==sector1||wall_sector==sd_split_sector)
            continue;

         if(map->walls[j].x==map->walls[swall].x&&map->walls[j].y==map->walls[swall].y)
         {
            if(map->walls[next].x==map->walls[sprev].x&&map->walls[next].y==map->walls[sprev].y)
            {
               map->walls[sprev].portal = wall_sector;
               map->walls[j].portal = sector1;
               map->walls[sprev].portal_wall = j;
               map->walls[j].portal_wall = sprev;
            }
         }
      }
   }

   //Copy sector attributes
   map->sectors[sector0].floor = map->sectors[sd_split_sector].floor;
   map->sectors[sector0].ceiling  = map->sectors[sd_split_sector].ceiling;
   map->sectors[sector1].floor = map->sectors[sd_split_sector].floor;
   map->sectors[sector1].ceiling  = map->sectors[sd_split_sector].ceiling;

   RvR_port_sector_delete(map,sd_split_sector);

   /*for(int i = 0;i<map->wall_count;i++)
   {
      printf("%5d: (%5d %5d) %2d %2d %2d\n",i,map->walls[i].x/1024,map->walls[i].y/1024,map->walls[i].p2,map->walls[i].portal,RvR_port_wall_sector(map,i));
   }*/

   return 1;
}

static int sector_draw_connect()
{
   //Remove last wall, will be added again
   RvR_array_length_set(sd_walls,RvR_array_length(sd_walls)-1);
   int old_length = RvR_array_length(sd_walls);

   //Add the entire loop of the last wall
   //(wall removed above will be added again here)
   int start = sd_split_wall_end;
   int end = sd_split_wall_end;
   int cur = start;
   do
   {
      RvR_port_wall w = map->walls[cur];
      w.p2 = RvR_array_length(sd_walls)+1;
      RvR_array_push(sd_walls,w);

      cur = map->walls[cur].p2;
   }
   while(cur!=end);

   //Add first walls again, in reverse
   for(int i = 0;i<old_length;i++)
   {
      RvR_array_push(sd_walls,sd_walls[old_length-i]);
      sd_walls[RvR_array_length(sd_walls)-1].p2 = RvR_array_length(sd_walls);
      sd_walls[RvR_array_length(sd_walls)-1].portal = -1;
      sd_walls[RvR_array_length(sd_walls)-1].portal_wall = -1;
   }

   //Add the entire loop of the first wall
   start = sd_split_wall;
   end = sd_split_wall;
   cur = start;
   do
   {
      RvR_port_wall w = map->walls[cur];
      w.p2 = RvR_array_length(sd_walls)+1;
      if(map->walls[cur].p2==end)
         w.p2 = 0;
      RvR_array_push(sd_walls,w);

      cur = map->walls[cur].p2;
   }
   while(cur!=end);

   //Add all other subsectors
   int sub_split = RvR_port_wall_subsector(map,sd_split_sector,sd_split_wall);
   int last_copied = sub_split;
   int subsector = 0;
   int sub_start = RvR_port_wall_subsector(map,sd_split_sector,sd_split_wall);
   int sub_end = RvR_port_wall_subsector(map,sd_split_sector,sd_split_wall_end);
   int split1_start = 0;
   for(int i = map->sectors[sd_split_sector].wall_first;i<map->sectors[sd_split_sector].wall_first+map->sectors[sd_split_sector].wall_count;i++)
   {
      if(subsector!=last_copied&&subsector!=sub_start&&subsector!=sub_end)
      {
         last_copied = subsector;
         int old_len = RvR_array_length(sd_walls);

         int p2 = RvR_array_length(sd_walls);
         int start = i;
         int end = i;
         int cur = start;
         do
         {
            RvR_port_wall w = map->walls[cur];
            w.p2 = RvR_array_length(sd_walls)+1;
            if(map->walls[cur].p2==end)
               w.p2 = p2;
            RvR_array_push(sd_walls,w);

            cur = map->walls[cur].p2;
         }
         while(cur!=end);

         //if we come across the first subsector, it needs to be moved
         //to the front of the sd_walls array, since the outer
         //subsector must always be first
         //this is done by rotating the array to the right by old_len elements
         if(subsector==0)
         {
            int rot_amount = RvR_array_length(sd_walls)-old_len;
            int rot_start = RvR_array_length(sd_walls)-rot_amount;
            split1_start+=rot_amount;

            //Reverse all
            for(int j = 0;j<RvR_array_length(sd_walls)/2;j++)
            {
               RvR_port_wall tmp = sd_walls[j];
               sd_walls[j] = sd_walls[RvR_array_length(sd_walls)-j-1];
               sd_walls[RvR_array_length(sd_walls)-j-1] = tmp;
            }

            //Reverse last rot_start
            for(int j = 0;j<rot_start/2;j++)
            {
               RvR_port_wall tmp = sd_walls[rot_amount+j];
               sd_walls[rot_amount+j] = sd_walls[RvR_array_length(sd_walls)-j-1];
               sd_walls[RvR_array_length(sd_walls)-j-1] = tmp;
            }

            //Reverse until rot_amount
            for(int j = 0;j<rot_amount/2;j++)
            {
               RvR_port_wall tmp = sd_walls[j];
               sd_walls[j] = sd_walls[rot_amount-j-1];
               sd_walls[rot_amount-j-1] = tmp;
            }

            //Adjust indices
            for(int j = 0;j<RvR_array_length(sd_walls);j++)
               sd_walls[j].p2 = (sd_walls[j].p2+rot_amount)%RvR_array_length(sd_walls);
         }
      }

      if(map->walls[i].p2<i)
         subsector++;
   }

   //Create new sector
   int16_t sector0 = map->sector_count++;
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
   map->sectors[sector0].wall_count = RvR_array_length(sd_walls);
   map->sectors[sector0].wall_first = map->wall_count;
   map->sectors[sector0].floor = 0;
   map->sectors[sector0].ceiling = 2*1024;
   map->sectors[sector0].floor_tex = 15;
   map->sectors[sector0].ceiling_tex = 15;
   map->wall_count+=RvR_array_length(sd_walls);
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
   for(int i = 0;i<RvR_array_length(sd_walls);i++)
   {
      map->walls[map->sectors[sector0].wall_first+i].x = sd_walls[i].x;
      map->walls[map->sectors[sector0].wall_first+i].y = sd_walls[i].y;
      map->walls[map->sectors[sector0].wall_first+i].p2 = sd_walls[i].p2+map->sectors[sector0].wall_first;
      map->walls[map->sectors[sector0].wall_first+i].flags = sd_walls[i].flags;
      map->walls[map->sectors[sector0].wall_first+i].shade_offset = sd_walls[i].shade_offset;
      map->walls[map->sectors[sector0].wall_first+i].x_off = sd_walls[i].x_off;
      map->walls[map->sectors[sector0].wall_first+i].y_off = sd_walls[i].y_off;
      map->walls[map->sectors[sector0].wall_first+i].tex_lower = sd_walls[i].tex_lower;
      map->walls[map->sectors[sector0].wall_first+i].tex_upper = sd_walls[i].tex_upper;
      map->walls[map->sectors[sector0].wall_first+i].tex_mid = sd_walls[i].tex_mid;
      map->walls[map->sectors[sector0].wall_first+i].portal = sd_walls[i].portal;
      map->walls[map->sectors[sector0].wall_first+i].portal_wall = -1;
   }

   //Fix links and portals
   for(int i = 0;i<map->sectors[sector0].wall_count;i++)
   {
      int16_t swall = map->sectors[sector0].wall_first+i;
      int16_t snext = RvR_port_wall_next(map,swall);
      int16_t sprev = RvR_port_wall_previous(map,swall);

      for(int j = 0;j<map->wall_count-map->sectors[sector0].wall_count;j++)
      {
         int16_t next = RvR_port_wall_next(map,j);
         int16_t prev = RvR_port_wall_previous(map,j);
         int16_t wall_sector = RvR_port_wall_sector(map,j);
         if(wall_sector==sd_split_sector)
            continue;

         if(map->walls[j].x==map->walls[swall].x&&map->walls[j].y==map->walls[swall].y)
         {
            if(map->walls[next].x==map->walls[sprev].x&&map->walls[next].y==map->walls[sprev].y)
            {
               map->walls[sprev].portal = wall_sector;
               map->walls[j].portal = sector0;
               map->walls[sprev].portal_wall = j;
               map->walls[j].portal_wall = sprev;
            }
         }
      }
   }

   //Copy sector attributes
   map->sectors[sector0].floor = map->sectors[sd_split_sector].floor;
   map->sectors[sector0].ceiling  = map->sectors[sd_split_sector].ceiling;

   //Fix sector windings
   RvR_port_sector_fix_winding(map,sector0);

   RvR_port_sector_delete(map,sd_split_sector);

   /*for(int i = 0;i<map->wall_count;i++)
   {
      printf("%5d: (%5d %5d) %2d %2d %2d\n",i,map->walls[i].x/1024,map->walls[i].y/1024,map->walls[i].p2,map->walls[i].portal,RvR_port_wall_sector(map,i));
   }*/

   return 1;
}
//-------------------------------------
