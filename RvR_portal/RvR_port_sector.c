/*
RvnicRaven - portal sectors

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
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
//-------------------------------------

//Function implementations

int RvR_port_sector_inside(const RvR_port_map *map, uint16_t sector, RvR_fix22 x, RvR_fix22 y)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_inside","argument 'map' must be non-NULL\n");
   RvR_error_check(sector!=RVR_PORT_SECTOR_INVALID,"RvR_port_sector_inside","invalid sector\n");
   RvR_error_check(sector<map->sector_count,"RvR_port_sector_inside","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);

   const RvR_port_sector *sec = &map->sectors[sector];

   //Even-odd rule for checking if inside
   int64_t crossed0 = 0;
   int64_t crossed1 = 0;
   for(int i = 0;i<sec->wall_count;i++)
   {
      //Translate, so that (x,y) is at (0,0)
      //--> easier comparisons later
      RvR_port_wall *w0 = map->walls+sec->wall_first+i;
      RvR_port_wall *w1 = map->walls+w0->p2;
      int64_t x0 = w0->x-x;
      int64_t y0 = w0->y-y;
      int64_t x1 = w1->x-x;
      int64_t y1 = w1->y-y;

      //We count the amount of walls on the line y = 0, incrementing
      //for every wall left of x = 0

      //Exactly on wall
      if((x0==0&&y0==0)||(x1==0&&y1==0))
         return 1;

      //If signs are not equal, then one of the points
      //is above (x,y), the other below
      if(!RvR_sign_equal(y0,y1))
      {
         //If both on one side
         if(RvR_sign_equal(x0,x1))
            //If both to the left --> increment
            crossed0+=x0<0;
         else
            //One of the x coordinates on the right of line, the other on the left
            //--> more complicated check
            //we use the 2d cross product/perp doct product
            //to compute on which side of the line we are
            //the result is dependant on the sign of y1 (or y0)
            //because we don't know, which of the two coordinates is the upper one
            crossed0+=!RvR_sign_equal(x0*y1-x1*y0,y1);
      }

      //Same thing again, but with
      //half-open interval
      x0--;
      y0--;
      x1--;
      y1--;
      if(!RvR_sign_equal(y0,y1))
      {

         if(RvR_sign_equal(x0,x1))
            crossed1+=x0<0;
         else
            crossed1+=!RvR_sign_equal(x0*y1-x1*y0,y1);
      }
   }

   //If we've corrsed an odd number of walls
   //on one of the counting rules, we are
   //inside the sector
   return crossed0&1||crossed1&1;

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
//-------------------------------------
