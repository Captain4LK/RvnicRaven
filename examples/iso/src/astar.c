/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "world_defs.h"
#include "astar.h"
#include "tile.h"
#include "entity.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   uint16_t cost;
   uint8_t dir;
   uint8_t visited;
} AS_node;

typedef struct
{
   uint32_t index;
   uint32_t cost;
} AS_heapnode;

typedef struct
{
   int16_t x;
   int16_t y;
   int16_t z;
} AS_pos;
//-------------------------------------

//Variables
static AS_node *nodes = NULL;
static AS_heapnode *heap = NULL;
static int heap_count = 0;
//-------------------------------------

//Function prototypes
static void as_heap_push(uint32_t index, uint32_t cost);
static void as_heap_pop(void);
//-------------------------------------

//Function implementations

void astar_init(Area *a)
{
   if(nodes!=NULL)
      RvR_free(nodes);
   nodes = RvR_malloc(sizeof(*nodes) * a->dimx*32 * a->dimy*32 * a->dimz*32, "A* nodes");

   if(heap==NULL)
      heap = RvR_malloc(sizeof(*heap) * ASTAR_HEAP_SIZE, "A* heap");
}

uint8_t *astar_path(Area *a, Entity *e, int dst_x, int dst_y, int dst_z, uint32_t *path_len)
{
   const AS_pos dir[26] = { { 1, 0 ,0}, { 0, 1 ,0}, { -1, 0 ,0}, { 0, -1 ,0}, { -1, 1 ,0}, { -1, -1 ,0}, { 1, -1 ,0}, { 1, 1 ,0}, {0,0,1}, {0,0,-1},
    { 1, 0 ,-1}, { 0, 1 ,-1}, { -1, 0 ,-1}, { 0, -1 ,-1}, { -1, 1 ,-1}, { -1, -1 ,-1}, { 1, -1 ,-1}, { 1, 1 ,-1},
    { 1, 0 ,1}, { 0, 1 ,1}, { -1, 0 ,1}, { 0, -1 ,1}, { -1, 1 ,1}, { -1, -1 ,1}, { 1, -1 ,1}, { 1, 1 ,1}};

   memset(nodes, 0, sizeof(*nodes) * a->dimx*32 * a->dimy*32 *a->dimz*32);
   heap_count = 0;
   AS_pos p = { .x = e->x, .y = e->y , .z = e->z};
   unsigned nindex = p.z*a->dimy*32*a->dimx*32+p.y * a->dimx*32+ p.x;
   AS_node *n = nodes + nindex;
   n->cost = 0;
   n->visited = 1;
   as_heap_push(nindex, RvR_max(RvR_abs(p.x-dst_x),RvR_max(RvR_abs(p.y-dst_y),RvR_abs(p.z-dst_z))));

   int found = 0;
   while(heap_count != 0)
   {
      nindex = heap[0].index;
      n = nodes + nindex;
      as_heap_pop();

      p.z = (int16_t)(nindex/(a->dimx*32*a->dimy*32));
      p.y = (int16_t)((nindex-p.z*a->dimx*32*a->dimy*32)/(a->dimx*32));
      p.x = (int16_t)(nindex%(a->dimx*32));
      if(p.x == dst_x && p.y == dst_y&& p.z ==dst_z)
      {
         found = 1;
         break;
      }
      n->visited = 1;

      for(unsigned i = 0; i < 8; i++)
      {
         AS_pos next = p;
         next.x += dir[i].x;
         next.y += dir[i].y;

         if(entity_pos_valid(a,e, next.x, next.y,next.z))
         {
            unsigned nnext_index = next.z*a->dimx*32*a->dimy*32+next.y*a->dimx*32+next.x;
            //unsigned nnext_index = next.y * map.width + next.x;
            AS_node *nnext = nodes + nnext_index;
            unsigned cost = n->cost + 1;

            if(!nnext->visited || cost < nnext->cost)
            {
               //Unlikely to reach a cost of 2047 anyway, so whatever
               nnext->cost = (cost)&((1<<11)-1);
               nnext->dir = i&((1<<4)-1);
               nnext->visited = 1;
               if(heap_count == ASTAR_HEAP_SIZE)
               {
                  RvR_log_line("astar_path", "out of memory\n");
                  goto bail;
               }

               unsigned new_cost = cost + RvR_max(RvR_abs(next.x-dst_x),RvR_max(RvR_abs(next.y-dst_y),RvR_abs(next.z-dst_z)));
               //unsigned new_cost = cost + 5 * (abs(next.x - dst_x) + (abs(next.y - dst_y)));
               as_heap_push(nnext_index, new_cost);
            }
         }
         //Upslope
         else if((tile_is_slope(area_tile(a, p.x, p.y, p.z))&&
                 tile_has_wall(area_tile(a, next.x, next.y, p.z))&&
                 !tile_has_wall(area_tile(a, next.x, next.y, p.z - 1))))
         {
            next.z--;

            unsigned nnext_index = next.z*a->dimx*32*a->dimy*32+next.y*a->dimx*32+next.x;
            //unsigned nnext_index = next.y * map.width + next.x;
            AS_node *nnext = nodes + nnext_index;
            unsigned cost = n->cost + 1;

            if(!nnext->visited || cost < nnext->cost)
            {
               //Unlikely to reach a cost of 2047 anyway, so whatever
               nnext->cost = (cost)&((1<<11)-1);
               nnext->dir = i+10;
               nnext->visited = 1;
               if(heap_count == ASTAR_HEAP_SIZE)
               {
                  RvR_log_line("astar_path", "out of memory\n");
                  goto bail;
               }

               unsigned new_cost = cost + RvR_max(RvR_abs(next.x-dst_x),RvR_max(RvR_abs(next.y-dst_y),RvR_abs(next.z-dst_z)));
               //unsigned new_cost = cost + 5 * (abs(next.x - dst_x) + (abs(next.y - dst_y)));
               as_heap_push(nnext_index, new_cost);
            }
         }
         //Downslope
         else if(tile_is_slope(area_tile(a, p.x, p.y, p.z + 1))&&
                 !tile_has_wall(area_tile(a, next.x, next.y, p.z + 1))&&
                 !tile_has_wall(area_tile(a, next.x, next.y, p.z)))
         {
            next.z++;

            unsigned nnext_index = next.z*a->dimx*32*a->dimy*32+next.y*a->dimx*32+next.x;
            //unsigned nnext_index = next.y * map.width + next.x;
            AS_node *nnext = nodes + nnext_index;
            unsigned cost = n->cost + 1;

            if(!nnext->visited || cost < nnext->cost)
            {
               //Unlikely to reach a cost of 2047 anyway, so whatever
               nnext->cost = (cost)&((1<<11)-1);
               nnext->dir = i+18;
               nnext->visited = 1;
               if(heap_count == ASTAR_HEAP_SIZE)
               {
                  RvR_log_line("astar_path", "out of memory\n");
                  goto bail;
               }

               unsigned new_cost = cost + RvR_max(RvR_abs(next.x-dst_x),RvR_max(RvR_abs(next.y-dst_y),RvR_abs(next.z-dst_z)));
               //unsigned new_cost = cost + 5 * (abs(next.x - dst_x) + (abs(next.y - dst_y)));
               as_heap_push(nnext_index, new_cost);
            }
         }
      }
   }
bail:
   if(found)
   {
      AS_pos po = p;
      AS_node *no = n;

      //Count
      *path_len = 0;
      while(p.x != e->x || p.y != e->y||p.z!=e->z)
      {
         p.x -= dir[n->dir].x;
         p.y -= dir[n->dir].y;
         p.z -= dir[n->dir].z;
         n = nodes + (p.z*a->dimx*32*a->dimy*32+p.y*a->dimx*32+p.x);
         (*path_len)++;
      }

      if(*path_len==0)
         return NULL;

      uint8_t *path = RvR_malloc(sizeof(*path) * (*path_len), "A* path");
      p = po;
      n = no;

      int index = (*path_len) - 1;
      while(p.x != e->x || p.y != e->y|| p.z!=e->z)
      {
         int dir_path = n->dir;
         if(dir_path>=18)
            dir_path-=18;
         else if(dir_path>=10)
            dir_path-=10;
         path[index--] = dir_path;
         p.x -= dir[n->dir].x;
         p.y -= dir[n->dir].y;
         p.z -= dir[n->dir].z;
         n = nodes + (p.z*a->dimx*32*a->dimy*32+p.y*a->dimx*32+p.x);
      }

      return path;
   }

   //No path found
   *path_len = 0;
   return NULL;
}

static void as_heap_push(uint32_t index, uint32_t cost)
{
   heap[heap_count].index = index;
   heap[heap_count].cost = cost;
   int i = heap_count++;
   int p = (i - 1) / 2;
   while(i != 0 && heap[p].cost > heap[i].cost)
   {
      AS_heapnode t = heap[i];
      heap[i] = heap[p];
      heap[p] = t;
      i = p;
      p = (i - 1) / 2;
   }
}

static void as_heap_pop(void)
{
   heap[0] = heap[--heap_count];
   int i = 0;
   for(;;)
   {
      int l = 2 * i + 1;
      int r = 2 * i + 2;
      int s = i;
      if(l < heap_count && heap[l].cost < heap[i].cost) s = l;
      if(r < heap_count && heap[r].cost < heap[s].cost) s = r;
      if(s == i) break;

      AS_heapnode t = heap[i];
      heap[i] = heap[s];
      heap[s] = t;
      i = s;
   }
}
//-------------------------------------
