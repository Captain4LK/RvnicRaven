/*
RvnicRaven-portal

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR_port_config.h"
#include "RvR/RvR_portal.h"
#include "RvR_port_render.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static int port_sprite_comp(const void *a, const void *b);
static int port_sprite_can_back(const port_sprite *a, const port_sprite *b);
static int port_wsprite_can_back(const port_sprite * restrict a, const port_sprite * restrict b);
//-------------------------------------

//Function implementations

void  RvR_port_draw_sprite(uint16_t sprite, void *ref)
{
   //flagged as invisible
   RvR_port_sprite *s = port_map->sprites+sprite;
   if(s->flags & RVR_PORT_SPRITE_INVISIBLE)
      return;

   //sector not visited
   if(!port_map->sectors[s->sector].visited)
      return;

   port_sprite sp = {0};
   sp.flags = s->flags;
   sp.texture = s->tex;
   sp.sprite = sprite;
   sp.x = s->x;
   sp.y = s->y;
   sp.z = s->z;
   sp.sector = s->sector;
   sp.dir = s->dir;
   sp.ref = ref;

   RvR_texture *tex = RvR_texture_get(s->tex);
   RvR_fix22 sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 cos = RvR_fix22_cos(port_cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres() * fovx, RvR_xres()*1024);
   RvR_fix22 sin_fov = RvR_fix22_mul(sin, fovx);
   RvR_fix22 cos_fov = RvR_fix22_mul(cos, fovx);
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;

   //Wall aligned
   if(s->flags & RVR_PORT_SPRITE_WALL)
   {
      //Translate sprite to world space
      RvR_fix22 dirx = RvR_fix22_cos(s->dir);
      RvR_fix22 diry = RvR_fix22_sin(s->dir);
      RvR_fix22 half_width = (tex->width * 1024) / (64 * 2);
      RvR_fix22 p0x = RvR_fix22_mul(-diry, half_width) + s->x;
      RvR_fix22 p0y = RvR_fix22_mul(dirx, half_width) + s->y;
      RvR_fix22 p1x = RvR_fix22_mul(diry, half_width) + s->x;
      RvR_fix22 p1y = RvR_fix22_mul(-dirx, half_width) + s->y;
      sp.x = s->x;
      sp.y = s->y;
      sp.z = s->z;
      sp.as.wall.u0 = 0;
      sp.as.wall.u1 = 1024 * tex->width - 1;

      //Translate to camera space
      RvR_fix22 x0 = p0x - port_cam->x;
      RvR_fix22 y0 = p0y - port_cam->y;
      RvR_fix22 x1 = p1x - port_cam->x;
      RvR_fix22 y1 = p1y - port_cam->y;
      RvR_fix22 tmp = x0;
      x0 = RvR_fix22_mul(-x0, sin) + RvR_fix22_mul(y0, cos);
      y0 = RvR_fix22_mul(tmp, cos_fov) + RvR_fix22_mul(y0, sin_fov);
      tmp = x1;
      x1 = RvR_fix22_mul(-x1, sin) + RvR_fix22_mul(y1, cos);
      y1 = RvR_fix22_mul(tmp, cos_fov) + RvR_fix22_mul(y1, sin_fov);

      //Behind camera
      if(y0<1&&y1<1)
         return;

      //Sprite not facing camera
      //--> swap p0 and p1 and toggle y-axis mirror flag
      if(RvR_fix22_mul(x0, y1) - RvR_fix22_mul(x1, y0)>0)
      {
         //One sided sprite
         if(sp.flags & RVR_PORT_SPRITE_ONESIDED)
            return;

         tmp = x0;
         x0 = x1;
         x1 = tmp;

         tmp = y0;
         y0 = y1;
         y1 = tmp;
         sp.flags ^= RVR_PORT_SPRITE_XFLIP;
      }

      sp.as.wall.xw0 = x0;
      sp.as.wall.zw0 = y0;
      sp.as.wall.xw1 = x1;
      sp.as.wall.zw1 = y1;

      //left point right of fov --> completely out of sight
      if((x0>=-y0||x1>y1)&&x0>y0)
         return;

      //right point left of fov --> completely out of sight
      if((x0<-y0||x1<=y1)&&x1<-y1)
         return;

      //p0
      //-------------------------------------
      sp.as.wall.x0 = RvR_min(RvR_xres() * 512+ RvR_fix22_div(x0* (RvR_xres() / 2), RvR_non_zero(y0)), RvR_xres() * 1024);
      sp.as.wall.z0 = y0;

      //Left point left of fov --> needs clipping
      if(x0<-y0)
      {
         sp.as.wall.x0 = 0;
         RvR_fix22 dx0 = x1 - x0;
         RvR_fix22 dx1 = x0 + y0;
         sp.as.wall.z0 = RvR_fix22_div(RvR_fix22_mul(dx0, dx1), y1 - y0+ x1 - x0) - x0;
         sp.as.wall.u0 = sp.as.wall.u0 + RvR_fix22_div(RvR_fix22_mul(-x0- y0, sp.as.wall.u1 - sp.as.wall.u0), RvR_non_zero(x1- x0+ y1- y0));
      }
      //-------------------------------------

      //p1
      //-------------------------------------
      sp.as.wall.x1 = RvR_min(RvR_xres() * 512+ RvR_fix22_div(x1* (RvR_xres() / 2), RvR_non_zero(y1)), RvR_xres() * 1024);
      sp.as.wall.z1 = y1;

      //Right point right of fov --> needs clipping
      if(x1>y1)
      {
         RvR_fix22 dx0 = x1 - x0;
         RvR_fix22 dx1 = y0- x0;
         sp.as.wall.x1 = RvR_xres() * 1024;
         sp.as.wall.z1 = x0- RvR_fix22_div(RvR_fix22_mul(dx0, dx1), y1- y0- x1+ x0);
         sp.as.wall.u1 = RvR_fix22_div(RvR_fix22_mul(dx1, sp.as.wall.u1), RvR_non_zero(-y1+ y0+ x1- x0));
      }
      //-------------------------------------

      //If the wall somehow ends up having a
      //negativ width, skip it
      if(sp.as.wall.x0>sp.as.wall.x1)
         return;

      //Near clip
      if(sp.as.wall.z0<1||sp.as.wall.z1<1)
         return;

      sp.z_min = RvR_min(sp.as.wall.zw0, sp.as.wall.zw1);
      sp.z_max = RvR_max(sp.as.wall.zw0, sp.as.wall.zw1);

      RvR_array_push(port_sprites, sp);

      return;
   }

   //Floor alligned
   if(s->flags & RVR_PORT_SPRITE_FLOOR)
   {
      //World space coordinates, origin at camera
      RvR_fix22 scos = RvR_fix22_cos(s->dir);
      RvR_fix22 ssin = RvR_fix22_sin(s->dir);
      RvR_fix22 half_width = (tex->width * 1024) / (64 * 2);
      RvR_fix22 half_height = (tex->height * 1024) / (64 * 2);
      RvR_fix22 x0 = RvR_fix22_mul(-half_width, -ssin) + RvR_fix22_mul(-half_height, scos) + s->x - port_cam->x;
      RvR_fix22 y0 = RvR_fix22_mul(-half_width, scos) + RvR_fix22_mul(-half_height, ssin) + s->y - port_cam->y;
      RvR_fix22 x1 = RvR_fix22_mul(+half_width, -ssin) + RvR_fix22_mul(-half_height, scos) + s->x - port_cam->x;
      RvR_fix22 y1 = RvR_fix22_mul(+half_width, scos) + RvR_fix22_mul(-half_height, ssin) + s->y - port_cam->y;
      RvR_fix22 x2 = RvR_fix22_mul(+half_width, -ssin) + RvR_fix22_mul(+half_height, scos) + s->x - port_cam->x;
      RvR_fix22 y2 = RvR_fix22_mul(+half_width, scos) + RvR_fix22_mul(+half_height, ssin) + s->y - port_cam->y;
      RvR_fix22 x3 = RvR_fix22_mul(-half_width, -ssin) + RvR_fix22_mul(+half_height, scos) + s->x - port_cam->x;
      RvR_fix22 y3 = RvR_fix22_mul(-half_width, scos) + RvR_fix22_mul(+half_height, ssin) + s->y - port_cam->y;

      //Move to camera space
      sp.as.floor.x0 = RvR_fix22_mul(-x0, sin) + RvR_fix22_mul(y0, cos);
      sp.as.floor.z0 = RvR_fix22_mul(x0, cos_fov) + RvR_fix22_mul(y0, sin_fov);
      sp.as.floor.x1 = RvR_fix22_mul(-x1, sin) + RvR_fix22_mul(y1, cos);
      sp.as.floor.z1 = RvR_fix22_mul(x1, cos_fov) + RvR_fix22_mul(y1, sin_fov);
      sp.as.floor.x2 = RvR_fix22_mul(-x2, sin) + RvR_fix22_mul(y2, cos);
      sp.as.floor.z2 = RvR_fix22_mul(x2, cos_fov) + RvR_fix22_mul(y2, sin_fov);
      sp.as.floor.x3 = RvR_fix22_mul(-x3, sin) + RvR_fix22_mul(y3, cos);
      sp.as.floor.z3 = RvR_fix22_mul(x3, cos_fov) + RvR_fix22_mul(y3, sin_fov);

      sp.as.floor.wx = RvR_fix22_mul(-(s->x - port_cam->x), sin) + RvR_fix22_mul(s->y - port_cam->y, cos);
      sp.as.floor.wy = RvR_fix22_mul(s->x - port_cam->x, cos_fov) + RvR_fix22_mul(s->y - port_cam->y, sin_fov);

      RvR_fix22 depth_min = RvR_min(sp.as.floor.z0, RvR_min(sp.as.floor.z1, RvR_min(sp.as.floor.z2, sp.as.floor.z3)));
      RvR_fix22 depth_max = RvR_max(sp.as.floor.z0, RvR_max(sp.as.floor.z1, RvR_max(sp.as.floor.z2, sp.as.floor.z3)));

      //Near clip
      if(depth_max<128)
         return;

      //Far clip
      //TODO(Captain4LK): far clip once really far away (only a few/one pixels big)?
      //if(depth_min>RVR_RAY_MAX_STEPS * 65536)
         //return;

      sp.z_min = depth_min;
      sp.z_max = depth_max;

      RvR_array_push(port_sprites, sp);

      return;
   }

   //Billboard
   sp.as.bill.wx = RvR_fix22_mul(-(s->x - port_cam->x), sin) + RvR_fix22_mul(s->y - port_cam->y, cos);
   sp.as.bill.wy = RvR_fix22_mul(s->x - port_cam->x, cos_fov) + RvR_fix22_mul(s->y - port_cam->y, sin_fov);
   sp.z_min = sp.z_max = sp.as.bill.wy;
   RvR_fix22 depth = RvR_fix22_mul(s->x - port_cam->x, cos) + RvR_fix22_mul(s->y - port_cam->y, sin); //Separate depth, so that the near/far clip is not dependent on fov

   //Near clip
   if(depth<128)
      return;

   //Far clip
   //TODO(Captain4LK): far clip once really far away (only a few/one pixels big)?
   //if(depth>RVR_RAY_MAX_STEPS * 65536)
      //return;

   //Left of screen
   if(-sp.as.bill.wx - tex->width * 8>sp.as.bill.wy)
      return;

   //Right of screen
   if(sp.as.bill.wx - tex->width * 8>sp.as.bill.wy)
      return;

   if(s->flags&RVR_PORT_SPRITE_CENTER)
   {
      //Above screen
      if(middle_row * RvR_fix22_mul(depth, fovy)<(RvR_yres()/2) * (s->z - port_cam->z-tex->height*8))
         return;

      //Below screen
      if((middle_row - RvR_yres()) * RvR_fix22_mul(depth, fovy)>(RvR_yres()/2) * (s->z - port_cam->z + tex->height * 8))
         return;
   }
   else
   {
      //Above screen
      if(middle_row * RvR_fix22_mul(depth, fovy)<(RvR_yres()/2) * (s->z - port_cam->z))
         return;

      //Below screen
      if((middle_row - RvR_yres()) * RvR_fix22_mul(depth, fovy)>(RvR_yres()/2) * (s->z - port_cam->z + tex->height * 16))
         return;
   }

   RvR_array_push(port_sprites, sp);
}

void sprites_render(RvR_port_selection *select)
{
   //Sprites get sorted from back to front

   //First sprites get sorted by depth.
   //This is not accurate for wall sprites
   qsort(port_sprites, RvR_array_length(port_sprites), sizeof(*port_sprites), port_sprite_comp);

   //This is essentially Newells Algorithm,
   //If you know a faster way to do this, please
   //tell me.
   int len = (int)RvR_array_length(port_sprites);
   for(int i = 0; i<len; i++)
   {
      //swaps is used to mark which sprites have been swapped before
      //and thus for detecting double swaps
      int swaps = 0;
      int j = i + 1;
      while(j<len)
      {
         //ray_sprite_can_back calculates wether sprite i
         //can be drawn behind sprite j
         //and thus if they are ordered correctly.
         if(port_sprite_can_back(port_sprites + i, port_sprites + j))
         {
            j++;
         }
         else if(i + swaps>j)
         {
            //This case usually happens when walls intersect
            //Here we would split the wall,
            //but since intersecting walls aren't supported we just pretend nothing happended
            j++;
         }
         else
         {
            //Place sprite j in front of sprite i,
            //shifting everything else to the right.
            port_sprite tmp = port_sprites[j];
            for(int w = j; w>i; w--)
               port_sprites[w] = port_sprites[w - 1];
            port_sprites[i] = tmp;
            j = i + 1;
            swaps++;
         }
      }
   }


   //Render sprites
   for(int i = 0; i<RvR_array_length(port_sprites); i++)
   {
      port_sprite *sp = port_sprites + i;
      if(sp->flags & RVR_PORT_SPRITE_WALL)
         port_sprite_draw_wall(sp, select);
      else if(sp->flags & RVR_PORT_SPRITE_FLOOR)
         port_sprite_draw_floor(sp, select);
      else
         port_sprite_draw_billboard(port_map, sp, select);
   }
}

static int port_sprite_comp(const void *a, const void *b)
{
   const port_sprite *sa = a;
   const port_sprite *sb = b;

   return sb->z_max - sa->z_max;
}

static int port_sprite_can_back(const port_sprite *a, const port_sprite *b)
{
   //Separate cases:
   //wall - wall : full check
   //sprite - sprite: nothing, only depth compare?
   //wall - sprite: partial check, only perp dot

   //Wall - Wall check is put in
   //a separate function
   if(a->flags & RVR_PORT_SPRITE_WALL&&b->flags & RVR_PORT_SPRITE_WALL)
      return port_wsprite_can_back(a, b);

   //Sprite - Sprite check
   //is a lot simpler
   if(!(a->flags & RVR_PORT_SPRITE_WALL)&&!(b->flags & RVR_PORT_SPRITE_WALL))
   {
      //If one is floor sprite, check height
      if(a->flags & RVR_PORT_SPRITE_FLOOR||b->flags & RVR_PORT_SPRITE_FLOOR)
      {
         //a completely behind b
         //We only want to sort them by height if
         //they overlap on the z-axis
         if(a->z_min>b->z_max)
            return 1;

         //If one of the sprites is above the camera, the higher
         //ones needs to be drawn first
         if(a->z>port_cam->z||b->z>port_cam->z)
            return (a->z)>=(b->z);
         //Otherwise, the lower one get's drawn first
         return (a->z)<=(b->z);
      }

      if(a->z_min>b->z_max)
         return 1;

      return 0;
   }

   int64_t x00 = 0;
   int64_t z00 = 0;
   int64_t x01 = 0;
   int64_t z01 = 0;
   int64_t x10 = 0;
   int64_t z10 = 0;
   int64_t x11 = 0;
   int64_t z11 = 0;
   if(a->flags & RVR_PORT_SPRITE_WALL)
   {
      x00 = a->as.wall.xw0;
      x01 = a->as.wall.xw1;
      z00 = a->as.wall.zw0;
      z01 = a->as.wall.zw1;
   }
   else if(a->flags & RVR_PORT_SPRITE_FLOOR)
   {
      x00 = a->as.floor.wx;
      x01 = a->as.floor.wx;
      z00 = a->as.floor.wy;
      z01 = a->as.floor.wy;
   }
   else
   {
      x00 = a->as.bill.wx;
      x01 = a->as.bill.wx;
      z00 = a->as.bill.wy;
      z01 = a->as.bill.wy;
   }

   if(b->flags & RVR_PORT_SPRITE_WALL)
   {
      x10 = b->as.wall.xw0;
      x11 = b->as.wall.xw1;
      z10 = b->as.wall.zw0;
      z11 = b->as.wall.zw1;
   }
   else if(b->flags & RVR_PORT_SPRITE_FLOOR)
   {
      x10 = b->as.floor.wx;
      x11 = b->as.floor.wx;
      z10 = b->as.floor.wy;
      z11 = b->as.floor.wy;
   }
   else
   {
      x10 = b->as.bill.wx;
      x11 = b->as.bill.wx;
      z10 = b->as.bill.wy;
      z11 = b->as.bill.wy;
   }

   //a completely behind b
   if(a->z_min>b->z_max)
      return 1;

   //can't check x overlap, since we don't
   //store/calculate the screen x coordinates.

   //One of the two is a wall
   //we check the ordering of
   //the wall and the sprite
   if(a->flags & RVR_PORT_SPRITE_WALL)
   {
      int64_t dx0 = x01 - x00;
      int64_t dz0 = z01 - z00;
      int64_t cross00 = dx0 * (z10 - z00) - dz0 * (x10 - x00);

      //sprite b in front wall a
      if(cross00<=0)
         return 1;
   }
   else
   {
      int64_t dx1 = x11 - x10;
      int64_t dz1 = z11 - z10;
      int64_t cross10 = dx1 * (z00 - z10) - dz1 * (x00 - x10);

      //sprite a behind wall b
      if(cross10>=0)
         return 1;
   }

   //Need swapping
   return 0;
}

static int port_wsprite_can_back(const port_sprite * restrict a, const port_sprite * restrict b)
{
   int64_t x00 = a->as.wall.xw0;
   int64_t z00 = a->as.wall.zw0;
   int64_t x01 = a->as.wall.xw1;
   int64_t z01 = a->as.wall.zw1;
   int64_t x10 = b->as.wall.xw0;
   int64_t z10 = b->as.wall.zw0;
   int64_t x11 = b->as.wall.xw1;
   int64_t z11 = b->as.wall.zw1;

   int64_t dx0 = x01-x00;
   int64_t dz0 = z01-z00;
   int64_t dx1 = x11-x10;
   int64_t dz1 = z11-z10;

   int64_t cross00 = dx0*(z10-z00)-dz0*(x10-x00);
   int64_t cross01 = dx0*(z11-z00)-dz0*(x11-x00);
   int64_t cross10 = dx1*(z00-z10)-dz1*(x00-x10);
   int64_t cross11 = dx1*(z01-z10)-dz1*(x01-x10);

   //wa completely behind wb
   if(RvR_min(z00,z01)>RvR_max(z10,z11))
      return 1;

   //no overlap on screen
   if(a->as.wall.x0>=b->as.wall.x1||a->as.wall.x1<=b->as.wall.x0)
      return 1;

   //p0 and p1 of wb in front of wa
   if(cross00<=0&&cross01<=0)
      return 1;

   //p0 and p1 of wa behind wb
   if(cross10>=0&&cross11>=0)
      return 1;
   
   //Need swapping
   return 0;
}
//-------------------------------------
