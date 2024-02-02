/*
RvnicRaven - portal sectors

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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

int RvR_port_sector_inside(const RvR_port_map *map, int16_t sector, RvR_fix22 x, RvR_fix22 y)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_inside","argument 'map' must be non-NULL\n");
   RvR_error_check(sector>=0,"RvR_port_sector_inside","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);
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

int16_t RvR_port_sector_update(const RvR_port_map *map, int16_t sector_last, RvR_fix22 x, RvR_fix22 y)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_update","argument 'map' must be non-NULL\n");

   if(sector_last>=0&&sector_last<map->sector_count)
   {
      //Check if still in same sector
      if(RvR_port_sector_inside(map,sector_last,x,y))
         return sector_last;

      //Check adjacent sectors
      //TODO(Captain4LK): higher depth search?
      for(int i = 0;i<map->sectors[sector_last].wall_count;i++)
      {
         int16_t portal = map->walls[map->sectors[sector_last].wall_first+i].portal;
         if(portal>=0&&RvR_port_sector_inside(map,portal,x,y))
            return portal;
      }
   }

   //Linear search over all sectors
   for(int i = 0;i<map->sector_count;i++)
      if(RvR_port_sector_inside(map,i,x,y))
         return i;

   //Not found, pretend we are still in the same sector
   return sector_last;

RvR_err:
   return -1;
}

//TODO(Captain4LK): remove this function, the editor doesn't need it
//so we can remove it once its done
int16_t RvR_port_sector_new(RvR_port_map *map, RvR_fix22 x, RvR_fix22 y)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_new","argument 'map' must be non-NULL\n");

   int16_t sector = map->sector_count++;
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
   map->sectors[sector].wall_count = 1;
   map->sectors[sector].wall_first = map->wall_count;
   map->wall_count++;
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
   map->walls[map->sectors[sector].wall_first].x = x;
   map->walls[map->sectors[sector].wall_first].y = y;
   map->walls[map->sectors[sector].wall_first].p2 = -1;
   map->walls[map->sectors[sector].wall_first].portal = -1;
   map->walls[map->sectors[sector].wall_first].portal_wall = -1;
   //map->walls[map->sectors[sector].wall_first].join = -1;

   return sector;

RvR_err:
   return -1;
}

void RvR_port_sector_fix_winding(RvR_port_map *map, int16_t sector)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_fix_winding","argument 'map' must be non-NULL\n");
   RvR_error_check(sector>=0,"RvR_port_sector_fix_winding","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);
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
         //Reverse walls
         for(int j = 0;j<(wall-first)/2;j++)
         {
            int16_t w0 = s->wall_first+first+j;
            int16_t w1 = s->wall_first+wall-j-1;

            RvR_port_wall tmp = map->walls[s->wall_first+first+j];
            int16_t pt20 = tmp.p2;
            int16_t pt21 = map->walls[s->wall_first+wall-j-1].p2;
            map->walls[s->wall_first+first+j] = map->walls[s->wall_first+wall-j-1];

            map->walls[s->wall_first+wall-j-1] = tmp;
            map->walls[s->wall_first+first+j].p2 = pt20;
            map->walls[s->wall_first+wall-j-1].p2 = pt21;
            
#if 0
            //Update joins
            int16_t cur = map->walls[w0].join;
            while(cur>=0&&cur!=w0)
            {
               if(map->walls[cur].join==w1)
               {
                  map->walls[cur].join = w0;
                  break;
               }
               cur = map->walls[cur].join;
            }
            cur = map->walls[w1].join;
            while(cur>=0&&cur!=w1)
            {
               if(map->walls[cur].join==w0)
               {
                  map->walls[cur].join = w1;
                  break;
               }
               cur = map->walls[cur].join;
            }
#endif
         }

         //Reverse all wall attributes except last
         //TODO(Captain4LK): fix portal_wall references
         for(int j = 0;j<(wall-first-1)/2;j++)
         {
            int16_t tmp = map->walls[s->wall_first+wall-j-2].portal;
            map->walls[s->wall_first+wall-j-2].portal = map->walls[s->wall_first+first+j].portal;
            map->walls[s->wall_first+first+j].portal = tmp;
         }
      }
   }

   return;

RvR_err:
   return;
}

int16_t RvR_port_sector_make_inner(RvR_port_map *map, int16_t wall)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_make_inner","argument 'map' must be non-NULL\n");
   RvR_error_check(wall>=0,"RvR_port_sector_make_inner","wall %d out of bounds (%d walls total)\n",wall,map->wall_count);
   RvR_error_check(wall<map->wall_count,"RvR_port_make_inner","wall %d out of bounds (%d walls total)\n",wall,map->wall_count);

   //TODO(Captain4LK): handle inner sector already partially being a portal

   int16_t sector = RvR_port_wall_sector(map,wall);
   int16_t first = RvR_port_wall_first(map,wall);

   int16_t sn = RvR_port_sector_new(map,map->walls[first].x,map->walls[first].y);
   map->sectors[sn].floor = map->sectors[sector].floor;
   map->sectors[sn].ceiling = map->sectors[sector].ceiling;
   map->walls[map->sectors[sn].wall_first].portal_wall = first;
   map->walls[first].portal_wall = map->sectors[sn].wall_first;
   //map->walls[map->sectors[sn].wall_first].join = first;
   //map->walls[first].join = map->sectors[sn].wall_first;
   map->walls[map->sectors[sn].wall_first].portal = sector;
   map->walls[first].portal = sn;

   int16_t w = map->walls[first].p2;
   while(w!=first)
   {
      int16_t wn = RvR_port_wall_append(map,sn,map->walls[w].x,map->walls[w].y);
      if(w>=wn) w++;
      if(first>=wn) first++;

      map->walls[wn].portal_wall = w;
      map->walls[w].portal_wall = wn;
      //map->walls[wn].join = w;
      //map->walls[w].join = wn;
      map->walls[wn].portal = sector;
      map->walls[w].portal = sn;
      w = map->walls[w].p2;
   }
   RvR_port_wall_append(map,sn,map->walls[first].x,map->walls[first].y);

   return sn;

RvR_err:
   return -1;
}

void RvR_port_sector_delete(RvR_port_map *map, int16_t sector)
{
   RvR_error_check(map!=NULL,"RvR_port_sector_delete","argument 'map' must be non-NULL\n");
   RvR_error_check(sector>=0,"RvR_port_sector_delete","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);
   RvR_error_check(sector<map->sector_count,"RvR_port_sector_delete","sector %d out of bounds (%d sectors total)\n",sector,map->sector_count);

   //Delete joins to walls in sector
   for(int w = map->sectors[sector].wall_first;w<map->sectors[sector].wall_first+map->sectors[sector].wall_count;w++)
   {
      RvR_port_wall *wall = map->walls+w;
#if 0
      if(wall->join>=0)
      {
         int16_t prev = RvR_port_wall_join_previous(map,w);
         if(prev==wall->join) //join list might only contain one element after removal
            map->walls[prev].join = -1;
         else
            map->walls[prev].join = wall->join;
      }
#endif
   }

   //Remove portals to sector
   for(int w = 0;w<map->wall_count;w++)
   {
      RvR_port_wall *wall = map->walls+w;
      if(wall->portal==sector)
         wall->portal = -1;
   }

   //Move walls after deleted sector to left
   int count = map->sectors[sector].wall_count;
   int remove = map->sectors[sector].wall_first;
   for(int w = remove;w<map->wall_count-count;w++)
      map->walls[w] = map->walls[w+count];

   //After moving the walls, some references (p2,join,wall_first) are incorrect, so...
   //...update wall references
   for(int w = 0;w<map->wall_count;w++)
   {
      RvR_port_wall *wall = map->walls+w;
      if(wall->p2>=remove)
         wall->p2-=count;
      if(wall->portal_wall>=remove)
         wall->portal_wall-=count;
      //if(wall->join>=remove)
         //wall->join-=count;
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
      if(wall->portal>=sector)
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
