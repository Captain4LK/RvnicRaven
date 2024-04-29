/*
RvnicRaven - iso roguelike

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
//-------------------------------------

//Internal includes
#include "area_draw.h"
#include "tile.h"
#include "area.h"
#include "camera.h"
#include "item_defs.h"
#include "entity_defs.h"
#include "entity.h"
#include "item.h"
#include "sprite.h"
#include "draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   Point pos;

   int type;
   union
   {
      Item *i;
      Entity *e;
   }as;
}Draw_sprite;
//-------------------------------------

//Variables
static const World *world;
static const Area *area;
static const Camera *cam;

static Draw_sprite *sprites = NULL;
//-------------------------------------

//Function prototypes
static void area_draw_sprite(Draw_sprite *s);

static int sprite_cmp_r0(const void *a, const void *b);
static int sprite_cmp_r1(const void *a, const void *b);
static int sprite_cmp_r2(const void *a, const void *b);
static int sprite_cmp_r3(const void *a, const void *b);
static int pos_cmp_r0(Point p0, Point p1);
static int pos_cmp_r1(Point p0, Point p1);
static int pos_cmp_r2(Point p0, Point p1);
static int pos_cmp_r3(Point p0, Point p1);
//-------------------------------------

//Function implementations

void area_draw_begin(const World *w, const Area *a, const Camera *c)
{
   world = w;
   area = a;
   cam = c;

   RvR_array_length_set(sprites, 0);
}

void area_draw_end()
{
   //Sort sprites
   switch(cam->rotation)
   {
   case 0: qsort(sprites, RvR_array_length(sprites), sizeof(*sprites), sprite_cmp_r0); break;
   case 1: qsort(sprites, RvR_array_length(sprites), sizeof(*sprites), sprite_cmp_r1); break;
   case 2: qsort(sprites, RvR_array_length(sprites), sizeof(*sprites), sprite_cmp_r2); break;
   case 3: qsort(sprites, RvR_array_length(sprites), sizeof(*sprites), sprite_cmp_r3); break;
   }

   int sprite_cur = 0;
   int sprite_max = (int)RvR_array_length(sprites);

   int cx = cam->x * 16 + cam->y * 16;
   int cy = cam->z * 20 - 8 * cam->x + 8 * cam->y;

   for(int z = area->dimz * 32 - 1; z>=0; z--)
   {
      int origin_y = (16 * cam->y - 20 * (z - cam->z)) / 16;
      int origin_x = -origin_y + cam->x + cam->y;
      int origin_z = z;
      int y = origin_y;

      //Would be infinite loop, but limited to prevent badness
      for(int i = 0; i<64; i++)
      {
         int min = RvR_max(-y + origin_x + origin_y, (20 * (z - origin_z) + 8 * (y - origin_y) - RvR_yres()) / 8 + origin_x);
         int max = RvR_min((RvR_xres() - 16 * (y - origin_y)) / 16 + origin_x, (20 * (z - origin_z) + 8 * (y - origin_y)) / 8 + origin_x);

         if(min>max)
            break;

         min = RvR_max(0, min - 1);
         if(cam->rotation==1||cam->rotation==3)
            max = RvR_min(area->dimy * 32, max + 2);
         else
            max = RvR_min(area->dimx * 32, max + 2);

         for(int x = max; x>=min; x--)
         {
            int tx = x;
            int ty = y;
            int txf = x;
            int tyr = y;
            switch(cam->rotation)
            {
            case 0: tx = x; ty = y; txf = tx - 1; tyr = ty + 1; break;
            case 1: tx = area->dimy * 32 - 1 - y; ty = x; txf = tx - 1; tyr = ty - 1; break;
            case 2: tx = area->dimx * 32 - 1 - x; ty = area->dimy * 32 - 1 - y; txf = tx + 1; tyr = ty - 1; break;
            case 3: tx = y; ty = area->dimy * 32 - 1 - x; txf = tx + 1; tyr = ty + 1; break;
            }

            //TODO(Captain4LK): rotate point function?
            uint32_t tile = area_tile(area, point(tx, ty, z));
            uint32_t front = area_tile(area, point(txf, ty, z));
            uint32_t right = area_tile(area, point(tx, tyr, z));
            uint32_t up = area_tile(area, point(tx, ty, z - 1));

            //if(area_tile(area,tx,ty,z)==tile_set_discovered(0,1))
            //continue;

            if(tile_has_draw_slope(tile))
            {
               RvR_texture *tex = RvR_texture_get(tile_slope_texture(tile, cam->rotation));
               if(tile_visible_wall(tile))
                  draw_sprite(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - 4 - cy);
               else
                  draw_sprite_bw(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - 4 - cy);
            }

            if(tile_discovered_wall(tile)&&tile_has_draw_wall(tile)&&(!tile_has_draw_floor(tile)||!tile_has_draw_wall(front)||!tile_has_draw_wall(right)||z==cam->z_cutoff))
            {
               RvR_texture *tex = RvR_texture_get(tile_wall_texture(tile));
               if(tile_visible_wall(tile))
                  draw_sprite(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - cy);
               else
                  draw_sprite_bw(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }

            //Draw_sprites
            if(sprite_cur<sprite_max)
            {
               Draw_sprite *sp = sprites + sprite_cur;
               if(point_equal(sp->pos, point(tx, ty, z)))
               {
                  RvR_texture *tex = NULL;
                  if(sp->type==0)
                  {
                     if(!sprite_valid(sp->as.e->sprite, sp->as.e))
                        entity_sprite_create(sp->as.e);

                     tex = RvR_texture_get(sprite_texture(sp->as.e->sprite));
                  }
                  else if(sp->type==1)
                  {
                     if(!sprite_valid(sp->as.i->sprite, sp->as.i))
                        item_sprite_create(sp->as.i);

                     tex = RvR_texture_get(sprite_texture(sp->as.i->sprite));
                  }

                  draw_sprite(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - cy - 4);
                  sprite_cur++;
               }
            }
            //Skip sprites until next
            switch(cam->rotation)
            {
            case 0: for(; sprite_cur<sprite_max&&pos_cmp_r0(sprites[sprite_cur].pos, point(tx, ty, z))<0; sprite_cur++); break;
            case 1: for(; sprite_cur<sprite_max&&pos_cmp_r1(sprites[sprite_cur].pos, point(tx, ty, z))<0; sprite_cur++); break;
            case 2: for(; sprite_cur<sprite_max&&pos_cmp_r2(sprites[sprite_cur].pos, point(tx, ty, z))<0; sprite_cur++); break;
            case 3: for(; sprite_cur<sprite_max&&pos_cmp_r3(sprites[sprite_cur].pos, point(tx, ty, z))<0; sprite_cur++); break;
            }

            if(z==cam->z_cutoff)
            {
               const int offs[4][4] =
               {
                  {0, -1, 1, 0},
                  {1, 0, 0, 1},
                  {0, 1, -1, 0},
                  {-1, 0, 0, -1},
               };

               int px = x * 16 + y * 16 - cx;
               int py = z * 20 - 8 * x + 8 * y - cy;
               int tx0 = tx + offs[cam->rotation & 3][0];
               int ty0 = ty + offs[cam->rotation & 3][1];
               int tx1 = tx + offs[cam->rotation & 3][2];
               int ty1 = ty + offs[cam->rotation & 3][3];

               if(tile_has_draw_wall(tile)&&!tile_has_draw_wall(area_tile(area, point(tx0, ty0, z))))
                  RvR_render_line((px) * 256 + 128, (py + 8) * 256 + 128, (px + 16) * 256 + 128, (py) * 256 + 128, 1);
               if(tile_has_draw_wall(tile)&&!tile_has_draw_wall(area_tile(area, point(tx1, ty1, z))))
                  RvR_render_line((px + 16) * 256 + 128, (py + 1) * 256 + 128, (px + 32) * 256 + 128, (py + 9) * 256 + 128, 1);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();

               continue;
            }

            if(tile_discovered_floor(tile)&&tile_has_draw_floor(tile)&&(!tile_has_draw_floor(front)||!tile_has_draw_floor(right)||!tile_has_draw_wall(up)))
            {
               RvR_texture *tex = RvR_texture_get(tile_floor_texture(tile));
               if(tile_visible_floor(tile))
                  draw_sprite(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - 4 - cy);
               else
                  draw_sprite_bw(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - 4 - cy);

               const int offs[4][4] =
               {
                  {0, -1, 1, 0},
                  {1, 0, 0, 1},
                  {0, 1, -1, 0},
                  {-1, 0, 0, -1},
               };

               int px = x * 16 + y * 16 - cx;
               int py = z * 20 - 8 * x + 8 * y - 4 - cy;
               int tx0 = tx + offs[cam->rotation & 3][0];
               int ty0 = ty + offs[cam->rotation & 3][1];
               int tx1 = tx + offs[cam->rotation & 3][2];
               int ty1 = ty + offs[cam->rotation & 3][3];

               if(!tile_has_draw_floor(area_tile(area, point(tx0, ty0, z))))
                  RvR_render_line((px) * 256 + 128, (py + 8) * 256 + 128, (px + 16) * 256 + 128, (py) * 256 + 128, 1);
               if(!tile_has_draw_floor(area_tile(area, point(tx1, ty1, z))))
                  RvR_render_line((px + 16) * 256 + 128, (py + 1) * 256 + 128, (px + 32) * 256 + 128, (py + 9) * 256 + 128, 1);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }
         }
         y++;
      }

      if(z==cam->z_cutoff)
         break;
   }
}

static void area_draw_sprite(Draw_sprite *s)
{
   int16_t x = s->pos.x;
   int16_t y = s->pos.y;
   int16_t z = s->pos.z;

   int dx = x;
   int dy = y;
   if(cam->rotation==1)
   {
      dx = y;
      dy = area->dimy * 32 - 1 - x;
   }
   else if(cam->rotation==2)
   {
      dx = area->dimx * 32 - 1 - x;
      dy = area->dimy * 32 - 1 - y;
   }
   else if(cam->rotation==3)
   {
      dx = area->dimy * 32 - 1 - y;
      dy = x;
   }

   //Out of bounds
   if(x<0||y<0||z<0)
      return;
   if(x>=area->dimx * 32||y>=area->dimy * 32||z>=area->dimz * 32)
      return;

   //Outside of screen
   RvR_texture *t = NULL;
   if(s->type==0)
   {
      if(!sprite_valid(s->as.e->sprite, s->as.e))
         entity_sprite_create(s->as.e);

      t = RvR_texture_get(sprite_texture(s->as.e->sprite));
   }
   else if(s->type==1)
   {
      if(!sprite_valid(s->as.i->sprite, s->as.i))
         item_sprite_create(s->as.i);

      t = RvR_texture_get(sprite_texture(s->as.i->sprite));
   }
   //RvR_texture *t = RvR_texture_get(tex);
   if(t==NULL)
      return;

   int cx = cam->x * 16 + cam->y * 16;
   int cy = cam->z * 20 - 8 * cam->x + 8 * cam->y;
   int sx = dx * 16 + dy * 16 - cx;
   int sy = z * 20 - 8 * dx + 8 * dy - cy;

   //Left of screen
   if(sx + t->width<0)
      return;

   //Right of screen
   if(sx>=RvR_xres())
      return;

   //Below screen
   if(sy>=RvR_yres())
      return;

   //Above screen
   if(sy + t->height<0)
      return;

   //Push into list
   Draw_sprite sp = *s;
   RvR_array_push(sprites, sp);
}

void area_draw_item(Item *it, Point pos)
{
   Draw_sprite sp = {0};
   sp.type = 1;
   sp.as.i = it;
   sp.pos = pos;

   area_draw_sprite(&sp);
}

void area_draw_entity(Entity *e, Point pos)
{
   Draw_sprite sp = {0};
   sp.type = 0;
   sp.as.e = e;
   sp.pos = pos;

   area_draw_sprite(&sp);
}

static int sprite_cmp_r0(const void *a, const void *b)
{
   const Draw_sprite *sa = a;
   const Draw_sprite *sb = b;

   return pos_cmp_r0(sa->pos, sb->pos);
}

static int sprite_cmp_r1(const void *a, const void *b)
{
   const Draw_sprite *sa = a;
   const Draw_sprite *sb = b;

   return pos_cmp_r1(sa->pos, sb->pos);
}

static int sprite_cmp_r2(const void *a, const void *b)
{
   const Draw_sprite *sa = a;
   const Draw_sprite *sb = b;

   return pos_cmp_r2(sa->pos, sb->pos);
}

static int sprite_cmp_r3(const void *a, const void *b)
{
   const Draw_sprite *sa = a;
   const Draw_sprite *sb = b;

   return pos_cmp_r3(sa->pos, sb->pos);
}

static int pos_cmp_r0(Point p0, Point p1)
{
   if(p0.z==p1.z)
   {
      if(p0.y==p1.y)
         return p1.x - p0.x;

      return p0.y - p1.y;
   }

   return p1.z - p0.z;
}

static int pos_cmp_r1(Point p0, Point p1)
{
   if(p0.z==p1.z)
   {
      if(p0.x==p1.x)
         return p1.y - p0.y;

      return p1.x - p0.x;
   }

   return p1.z - p0.z;
}

static int pos_cmp_r2(Point p0, Point p1)
{
   if(p0.z==p1.z)
   {
      if(p0.y==p1.y)
         return p0.x - p1.x;

      return p1.y - p0.y;
   }

   return p1.z - p0.z;
}

static int pos_cmp_r3(Point p0, Point p1)
{
   if(p0.z==p1.z)
   {
      if(p0.x==p1.x)
         return p0.y - p1.y;

      return p0.x - p1.x;
   }

   return p1.z - p0.z;
}
//-------------------------------------
