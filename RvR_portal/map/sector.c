/*
RvnicRaven-portal

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR/RvR_portal.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static int port_point_on_line(RvR_fix22 x, RvR_fix22 y, RvR_fix22 x0, RvR_fix22 y0, RvR_fix22 x1, RvR_fix22 y1);
//-------------------------------------

//Function implementations

int RvR_port_sector_inside(const RvR_port_map *map, uint16_t sector, RvR_fix22 x, RvR_fix22 y)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_inside","argument 'map' must be non-NULL\n");
   RvR_error_check(sector!=RVR_PORT_SECTOR_INVALID,"RvR_port_sector_inside","invalid sector\n");
   RvR_error_check(sector<map->sector_count,"RvR_port_sector_inside","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);

   const RvR_port_sector *sec = &map->sectors[sector];

   int crossed = 0;
   for(int i = 0;i<sec->wall_count;i++)
   {
      RvR_port_wall *w0 = map->walls+sec->wall_first+i;
      RvR_port_wall *w1 = map->walls+w0->p2;
      int64_t x0 = w0->x;
      int64_t y0 = w0->y;
      int64_t x1 = w1->x;
      int64_t y1 = w1->y;

      if(port_point_on_line(x,y,w0->x,w0->y,w1->x,w1->y))
         return 1;

      //Below point
      if(RvR_min(y0,y1)>=y)
         continue;

      //Above point
      if(RvR_max(y0,y1)<y)
         continue;

      //Left of point
      if(RvR_max(x0,x1)<x)
         continue;

      //Horizontal line
      if(y0==y1)
         continue;

      //Vertical line
      if(x0==x1)
      {
         crossed++;
         continue;
      }
      
      if(y1-y0>0)
      {
         if(x*(y1-y0)<=(y-y0)*(x1-x0)+x0*(y1-y0))
            crossed++;
      }
      else
      {
         if(x*(y1-y0)>=(y-y0)*(x1-x0)+x0*(y1-y0))
            crossed++;
      }
   }

   return crossed&1;

RvR_err:
   return 0;
}

uint16_t RvR_port_sector_update(const RvR_port_map *map, uint16_t sector_last, RvR_fix22 x, RvR_fix22 y)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_update","argument 'map' must be non-NULL\n");

   if(sector_last!=RVR_PORT_SECTOR_INVALID&&sector_last<map->sector_count)
   {
      //Check if still in same sector
      if(RvR_port_sector_inside(map,sector_last,x,y))
         return sector_last;

      //Check adjacent sectors
      //TODO(Captain4LK): higher depth search?
      for(int i = 0;i<map->sectors[sector_last].wall_count;i++)
      {
         uint16_t portal = map->walls[map->sectors[sector_last].wall_first+i].portal;
         if(portal!=RVR_PORT_SECTOR_INVALID&&RvR_port_sector_inside(map,portal,x,y))
            return portal;
      }
   }

   //Linear search over all sectors
   for(int i = 0;i<map->sector_count;i++)
      if(RvR_port_sector_inside(map,(uint16_t)i,x,y))
         return (uint16_t)i;

   //Not found, pretend we are still in the same sector
   return sector_last;

RvR_err:
   return RVR_PORT_SECTOR_INVALID;
}

void RvR_port_sector_fix_winding(RvR_port_map *map, uint16_t sector)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_fix_winding","argument 'map' must be non-NULL\n");
   RvR_error_check(sector!=RVR_PORT_SECTOR_INVALID,"RvR_port_sector_fix_winding","sector invalid\n",sector,map->sector_count);
   RvR_error_check(sector<map->sector_count,"RvR_port_sector_fix_winding","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);

   int wall = 0;
   RvR_port_sector *s = map->sectors+sector;
   while(wall<map->sectors[sector].wall_count-1)
   {
      int64_t sum = 0;
      int first = wall;

      //Calculate winding number, positive if clockwise
      for(;wall==first||map->walls[map->sectors[sector].wall_first+wall-1].p2==map->sectors[sector].wall_first+wall;wall++)
      {
         int64_t x0 = map->walls[s->wall_first+wall].x;
         int64_t y0 = map->walls[s->wall_first+wall].y;
         int64_t x1 = map->walls[map->walls[s->wall_first+wall].p2].x;
         int64_t y1 = map->walls[map->walls[s->wall_first+wall].p2].y;
         sum+=(x1-x0)*(y0+y1);
      }

      //First subsector must be clockwise, all other subsectors counter-clockwise
      if((first==0&&sum>0)||(first!=0&&sum<0))
      {
         printf("Reverse %d, %d to %d\n",sector,first,wall-1);

         //Rotate positions
         for(int j = 0;j<(wall-first-1)/2;j++)
         {
            RvR_fix22 tmp = map->walls[s->wall_first+first+1+j].x;
            map->walls[s->wall_first+first+1+j].x = map->walls[s->wall_first+wall-j-1].x;
            map->walls[s->wall_first+wall-j-1].x = tmp;

            tmp = map->walls[s->wall_first+first+1+j].y;
            map->walls[s->wall_first+first+1+j].y = map->walls[s->wall_first+wall-j-1].y;
            map->walls[s->wall_first+wall-j-1].y = tmp;
         }
         
         //Reverse everything else
         for(int j = 0;j<(wall-first)/2;j++)
         {
            uint16_t w0 = (uint16_t)(s->wall_first+first+j);
            uint16_t w1 = (uint16_t)(s->wall_first+wall-j-1);
            RvR_port_wall tmp = map->walls[w0];
            uint16_t pt20 = tmp.p2;
            uint16_t pt21 = map->walls[w1].p2;
            RvR_fix22 x0 = tmp.x;
            RvR_fix22 x1 = map->walls[w1].x;
            RvR_fix22 y0 = tmp.y;
            RvR_fix22 y1 = map->walls[w1].y;
            map->walls[w0] = map->walls[s->wall_first+wall-j-1];
            map->walls[w1] = tmp;
            map->walls[w0].p2 = pt20;
            map->walls[w1].p2 = pt21;
            map->walls[w0].x = x0;
            map->walls[w1].x = x1;
            map->walls[w0].y = y0;
            map->walls[w1].y = y1;

            //Update linked walls
            if(map->walls[w0].portal_wall!=RVR_PORT_WALL_INVALID)
               map->walls[map->walls[w0].portal_wall].portal_wall = w0;
            if(map->walls[w1].portal_wall!=RVR_PORT_WALL_INVALID)
               map->walls[map->walls[w1].portal_wall].portal_wall = w1;
         }
      }
   }

   return;

RvR_err:
   return;
}

uint16_t RvR_port_sector_make_inner(RvR_port_map *map, uint16_t wall)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_make_inner","argument 'map' must be non-NULL\n");
   RvR_error_check(wall!=RVR_PORT_WALL_INVALID,"RvR_port_sector_make_inner","wall invalid\n",wall,map->wall_count);
   RvR_error_check(wall<map->wall_count,"RvR_port_make_inner","wall %d out of bounds (%d walls total)\n",wall,map->wall_count);

   uint16_t sector_root = RvR_port_wall_sector(map,wall);

   //Add enough new walls to hold subsector
   //-------------------------------------
   uint16_t len = RvR_port_wall_onesided_length(map,wall);
   uint16_t first = map->wall_count;
   map->wall_count+=len;
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
   uint16_t sector = map->sector_count;
   map->sector_count++;
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sector grow");
   map->sectors[sector] = map->sectors[sector_root];
   map->sectors[sector].wall_count = len;
   map->sectors[sector].wall_first = first;
   //-------------------------------------

   //Copy subsector and add portals
   //-------------------------------------
   uint16_t cur = first;
   uint16_t cur_src = wall;
   map->walls[cur] = map->walls[wall];
   map->walls[cur].p2 = cur+1;
   map->walls[cur].portal = sector_root;
   map->walls[cur].portal_wall = wall;
   cur = map->walls[cur].p2;
   cur_src = RvR_port_wall_next_onesided(map,cur_src);
   while(cur_src!=wall)
   {
      map->walls[cur] = map->walls[cur_src];
      map->walls[cur].p2 = cur+1;
      map->walls[cur].portal = RvR_port_wall_sector(map,cur_src);
      map->walls[cur].portal_wall = cur_src;

      cur_src = RvR_port_wall_next_onesided(map,cur_src);

      if(cur_src==wall)
         map->walls[cur].p2 = first;

      cur = map->walls[cur].p2;
   }

   for(int i = 0;i<map->sectors[sector].wall_count;i++)
   {
      uint16_t w = map->sectors[sector].wall_first+(uint16_t)i;
      map->walls[map->walls[w].portal_wall].portal_wall = w;
      map->walls[map->walls[w].portal_wall].portal = sector;
   }
   //-------------------------------------

   printf("Check: %d\n",RvR_port_map_check(map));
   RvR_port_map_print_walls(map);
   RvR_port_sector_fix_winding(map,sector);
   printf("Check: %d\n",RvR_port_map_check(map));
   RvR_port_map_print_walls(map);

   return sector;

RvR_err:
   return RVR_PORT_SECTOR_INVALID;
}

void RvR_port_sector_delete(RvR_port_map *map, uint16_t sector)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_delete","argument 'map' must be non-NULL\n");
   RvR_error_check(sector!=RVR_PORT_SECTOR_INVALID,"RvR_port_sector_delete","sector invalid\n",sector,map->sector_count);
   RvR_error_check(sector<map->sector_count,"RvR_port_sector_delete","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);

   //Remove portals to sector
   for(int i = 0;i<map->sectors[sector].wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+map->sectors[sector].wall_first+i;
      if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&
         map->walls[wall->portal_wall].portal_wall==map->sectors[sector].wall_first+(uint16_t)i)
      {
         map->walls[wall->portal_wall].portal_wall = RVR_PORT_WALL_INVALID;
         map->walls[wall->portal_wall].portal = RVR_PORT_SECTOR_INVALID;
      }
      wall->portal_wall = RVR_PORT_WALL_INVALID;
      wall->portal = RVR_PORT_SECTOR_INVALID;
   }

   //Move walls after deleted sector to left
   uint16_t count = map->sectors[sector].wall_count;
   uint16_t remove = map->sectors[sector].wall_first;
   for(int w = remove;w<map->wall_count-count;w++)
      map->walls[w] = map->walls[w+count];

   //After moving the walls, some references (p2,portal_wall,wall_first) are incorrect, so...
   //...update wall references
   for(int w = 0;w<map->wall_count;w++)
   {
      RvR_port_wall *wall = map->walls+w;
      if(wall->p2>=remove)
         wall->p2-=count;
      if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=remove)
         wall->portal_wall-=count;
   }

   //...and update sector references
   for(int s = 0;s<map->sector_count;s++)
   {
      RvR_port_sector *sct = map->sectors+s;
      if(sct->wall_first>=remove)
         sct->wall_first-=count;
   }

   //Move sectors after deleted sector to left
   for(int s = sector;s<map->sector_count-1;s++)
      map->sectors[s] = map->sectors[s+1];

   //After moving portals, some portal references are incorrect, so...
   //...update portals
   for(int w = 0;w<map->wall_count;w++)
   {
      RvR_port_wall *wall = map->walls+w;
      if(wall->portal!=RVR_PORT_SECTOR_INVALID&&wall->portal>=sector)
         wall->portal-=1;
   }

   //Shrink wall and sector arrays in map
   map->wall_count-=count;
   map->sector_count-=1;
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");

   return;

RvR_err:
   return;
}

uint16_t RvR_port_sector_join(RvR_port_map *map, uint16_t sector0, uint16_t sector1)
{
   //Count shared walls
   uint16_t shared = 0;
   for(int i = 0;i<map->sectors[sector0].wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+map->sectors[sector0].wall_first+i;
      if(wall->portal==sector1)
         shared++;
   }

   if(shared==0)
      return RVR_PORT_SECTOR_INVALID;

   //Allocate walls for new sector
   uint16_t new = map->sector_count++;
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
   map->sectors[new] = map->sectors[sector0];
   map->sectors[new].wall_count = 0;
   map->sectors[new].wall_first = map->wall_count;
   uint16_t next_wall = map->wall_count;
   map->wall_count+=(uint16_t)(map->sectors[sector0].wall_count+map->sectors[sector1].wall_count-2*shared);
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");

   uint16_t sectors[2] = {sector0,sector1};
   uint8_t *collected[2] = {0};
   collected[0] = RvR_malloc(sizeof(*collected)*map->sectors[sector0].wall_count,"Sector_join collected");
   collected[1] = RvR_malloc(sizeof(*collected)*map->sectors[sector1].wall_count,"Sector_join collected");
   memset(collected[0],0,sizeof(*collected)*map->sectors[sector0].wall_count);
   memset(collected[1],0,sizeof(*collected)*map->sectors[sector1].wall_count);

   //Sector0
   for(int i = 0;i<map->sectors[sector0].wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+map->sectors[sector0].wall_first+i;

      if(collected[0][i])
         continue;

      if(wall->portal==sector1)
      {
         collected[0][i] = 1;
         continue;
      }

      //Copy loop
      uint16_t first_wall = next_wall;
      int cur_sector = 0;
      uint16_t cur = map->sectors[sectors[cur_sector]].wall_first+(uint16_t)i;
      do
      {
         map->walls[next_wall] = map->walls[cur];
         map->walls[next_wall].p2 = next_wall+1;
         map->sectors[new].wall_count++;
         next_wall++;

         collected[cur_sector][cur-map->sectors[sectors[cur_sector]].wall_first] = 1;

         cur = map->walls[cur].p2;
         if(map->walls[cur].portal==sectors[!cur_sector])
         {
            cur = map->walls[map->walls[cur].portal_wall].p2;
            cur_sector = !cur_sector;
         }
      }while(!collected[cur_sector][cur-map->sectors[sectors[cur_sector]].wall_first]&&map->walls[cur].portal!=sectors[!cur_sector]);

      map->walls[next_wall-1].p2 = first_wall;
   }

   //Sector1
   for(int i = 0;i<map->sectors[sector1].wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+map->sectors[sector1].wall_first+i;

      if(collected[1][i])
         continue;

      if(wall->portal==sector0)
      {
         collected[1][i] = 1;
         continue;
      }

      //Copy loop
      uint16_t first_wall = next_wall;
      int cur_sector = 1;
      uint16_t cur = map->sectors[sectors[cur_sector]].wall_first+(uint16_t)i;
      do
      {
         map->walls[next_wall] = map->walls[cur];
         map->walls[next_wall].p2 = next_wall+1;
         map->sectors[new].wall_count++;
         next_wall++;

         collected[cur_sector][cur-map->sectors[sectors[cur_sector]].wall_first] = 1;

         cur = map->walls[cur].p2;
         if(map->walls[cur].portal==sectors[!cur_sector])
         {
            cur = map->walls[map->walls[cur].portal_wall].p2;
            cur_sector = !cur_sector;
         }
      }while(!collected[cur_sector][cur-map->sectors[sectors[cur_sector]].wall_first]&&map->walls[cur].portal!=sectors[!cur_sector]);

      map->walls[next_wall-1].p2 = first_wall;
   }

   //Fix subsector ordering
   //Outer subsector might not be first anymore
   int subsector = 0;
   int last = -1;
   for(int i = 0;i<map->sectors[new].wall_count;i++)
   {
      uint16_t wall = map->sectors[new].wall_first+(uint16_t)i;
      int all_in = 1;

      if(last!=subsector)
      {
         last = subsector;

         int winding = RvR_port_wall_winding(map,wall);
         int inner = 0;
         int inner_last = -1;
         for(int j = 0;j<map->sectors[new].wall_count;j++)
         {
            uint16_t wall_inner = map->sectors[new].wall_first+(uint16_t)j;

            if(inner_last!=inner&&inner!=subsector)
            {
               inner_last = inner;

               int inside = RvR_port_wall_inside(map,wall,map->walls[wall_inner].x,map->walls[wall_inner].y);

               if(winding!=0||!inside)
               {
                  all_in = 0;
                  break;
               }
            }

            if(map->walls[map->sectors[new].wall_first+(uint16_t)j].p2<map->sectors[new].wall_first+(uint16_t)j)
               inner++;
         }

         if(all_in)
         {
            //Rotate to be first
            uint16_t first = map->sectors[new].wall_first;
            uint16_t count = map->sectors[new].wall_count;
            int rot_amount = count-(wall-first);
            int rot_start = map->sectors[new].wall_count-rot_amount;

            //Reverse all
            for(int j = 0;j<map->sectors[new].wall_count/2;j++)
            {
               RvR_port_wall tmp = map->walls[first+j];
               map->walls[first+j] = map->walls[first+count-j-1];
               map->walls[first+count-j-1] = tmp;
            }

            //Reverse last rot_start
            for(int j = 0;j<rot_start/2;j++)
            {
               RvR_port_wall tmp = map->walls[first+rot_amount+j];
               map->walls[first+rot_amount+j] = map->walls[first+count-j-1];
               map->walls[first+count-j-1] = tmp;
            }

            //Reverse until rot_amount
            for(int j = 0;j<rot_amount/2;j++)
            {
               RvR_port_wall tmp = map->walls[first+j];
               map->walls[first+j] = map->walls[first+rot_amount-j-1];
               map->walls[first+rot_amount-j-1] = tmp;
            }

            //Fix indices
            for(int j = 0;j<map->sectors[new].wall_count;j++)
               map->walls[first+j].p2 = (uint16_t)((map->walls[first+j].p2-first+rot_amount)%map->sectors[new].wall_count+first);

            break;
         }
      }

      if(map->walls[map->sectors[new].wall_first+(uint16_t)i].p2<map->sectors[new].wall_first+(uint16_t)i)
         subsector++;
   }

   //Fix portals
   for(int i = 0;i<map->sectors[new].wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+map->sectors[new].wall_first+i;
      if(wall->portal_wall!=RVR_PORT_WALL_INVALID)
      {
         map->walls[wall->portal_wall].portal_wall = map->sectors[new].wall_first+(uint16_t)i;
         map->walls[wall->portal_wall].portal = new;
      }
   }

   //Delete old sectors
   if(sector0>sector1)
   {
      RvR_port_sector_delete(map,sector0);
      RvR_port_sector_delete(map,sector1);
   }
   else
   {
      RvR_port_sector_delete(map,sector1);
      RvR_port_sector_delete(map,sector0);
   }
   RvR_free(collected[0]);
   RvR_free(collected[1]);

   RvR_port_sector_fix_winding(map,map->sector_count-1);

   return map->sector_count-1;
}

RvR_fix22 RvR_port_sector_floor_at(const RvR_port_map *map, uint16_t sector, RvR_fix22 x, RvR_fix22 y)
{
   RvR_fix22 floor = map->sectors[sector].floor;
   if(map->sectors[sector].slope_floor!=0)
   {
      RvR_port_slope slope;
      RvR_port_slope_from_floor(map,sector,&slope);
      floor = RvR_port_slope_height_at(&slope,x,y);
   }

   return floor;
}

RvR_fix22 RvR_port_sector_ceiling_at(const RvR_port_map *map, uint16_t sector, RvR_fix22 x, RvR_fix22 y)
{
   RvR_fix22 ceiling = map->sectors[sector].ceiling;
   if(map->sectors[sector].slope_ceiling!=0)
   {
      RvR_port_slope slope;
      RvR_port_slope_from_ceiling(map,sector,&slope);
      ceiling = RvR_port_slope_height_at(&slope,x,y);
   }

   return ceiling;
}

static int port_point_on_line(RvR_fix22 x, RvR_fix22 y, RvR_fix22 x0, RvR_fix22 y0, RvR_fix22 x1, RvR_fix22 y1)
{
   //If 2d cross product is zero, point is on line
   int64_t cross = (int64_t)(x-x0)*(y1-y0)-(int64_t)(y-y0)*(x1-x0);
   if(cross!=0)
      return 0;

   //Point on line, but may not be inbetween p0 and p1
   if(RvR_abs(x1-x0)>=RvR_abs(y1-y0))
   {
      if(x1-x0>0)
         return x0<=x&&x<=x1;
      return x1<=x&&x<=x0;
   }
   else
   {
      if(y1-y0>0)
         return y0<=y&&y<=y1;
      return y1<=y&&y<=y0;
   }
}
//-------------------------------------
