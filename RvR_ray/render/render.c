/*
RvnicRaven - raycast drawing

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
#include "RvR_ray_config.h"
#include "RvR/RvR_ray.h"
#include "RvR_ray_render.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct ray_plane ray_plane;

struct ray_plane
{
   int32_t min;
   int32_t max;
   RvR_fix16 height;
   uint16_t tex;
   uint16_t start[RVR_XRES_MAX + 2];
   uint16_t end[RVR_XRES_MAX + 2];

   ray_plane *next;
};
//-------------------------------------

//Variables

static RvR_fix16 ray_span_start[RVR_YRES_MAX] = {0};

static ray_plane *ray_planes[128] = {0};
static ray_plane *ray_plane_pool = NULL;

RvR_ray_depth_buffer ray_depth_buffer;
RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_pool = NULL;

const RvR_ray_cam *ray_cam = NULL;
const RvR_ray_map *ray_map = NULL;
//-------------------------------------

//Function prototypes
static void ray_draw_column(const RvR_ray_map *map, const RvR_ray_cam *cam, RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray);
static int16_t ray_draw_wall(const RvR_ray_map *map, const RvR_ray_cam *cam, RvR_fix16 y_current, RvR_fix16 y_from, RvR_fix16 y_to, RvR_fix16 limit0, RvR_fix16 limit1, RvR_fix16 height, int16_t increment, RvR_ray_pixel_info *pixel_info, RvR_ray_hit_result *hit);
static void ray_span_draw_tex(const RvR_ray_cam *cam, int x0, int x1, int y, RvR_fix16 height, const RvR_texture *texture);

static void ray_plane_add(RvR_fix16 height, uint16_t tex, int x, int y0, int y1);

static RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_new();
static void ray_depth_buffer_entry_free(RvR_ray_depth_buffer_entry *ent);

static ray_plane *ray_plane_new();
static void ray_plane_free(ray_plane *pl);
//-------------------------------------

//Function implementations

void RvR_ray_draw_begin(const RvR_ray_cam *cam, const RvR_ray_map *map)
{
   ray_cam = cam;
   ray_map = map;

   //Clear depth buffer
   for(int i = 0; i<RvR_xres(); i++)
   {
      ray_depth_buffer_entry_free(ray_depth_buffer.floor[i]);
      ray_depth_buffer_entry_free(ray_depth_buffer.ceiling[i]);

      ray_depth_buffer.floor[i] = NULL;
      ray_depth_buffer.ceiling[i] = NULL;
   }

   //Clear planes
   for(int i = 0; i<128; i++)
   {
      ray_plane_free(ray_planes[i]);
      ray_planes[i] = NULL;
   }

   RvR_array_length_set(ray_sprites, 0);
}

void RvR_ray_draw_end(const RvR_ray_cam *cam, const RvR_ray_map *map, RvR_ray_selection *select)
{
   sprites_render(select);
}

RvR_ray_pixel_info RvR_ray_map_to_screen(const RvR_ray_cam *cam, RvR_fix16 x, RvR_fix16 y, RvR_fix16 z)
{
   RvR_ray_pixel_info result;

   RvR_fix16 to_pointx = x - cam->x;
   RvR_fix16 to_pointy = y - cam->y;
   RvR_fix16 cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov / 2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres() * fovx * 2, RvR_xres() << 16);

   RvR_fix16 middle_column = RvR_xres() / 2;

   //rotate the point to camera space (y left/right, x forw/backw)
   RvR_fix16 tmp = to_pointx;
   to_pointx = RvR_fix16_mul(to_pointx, cos) + RvR_fix16_mul(to_pointy, sin);
   to_pointy = RvR_fix16_mul(tmp, sin) + RvR_fix16_mul(to_pointy, cos);

   result.depth = to_pointx;

   result.x = middle_column - (to_pointy * middle_column) / RvR_non_zero(RvR_abs(RvR_fix16_mul(fovx, result.depth)));
   result.y = ((z - cam->z) * RvR_yres()) / RvR_non_zero(RvR_abs(RvR_fix16_mul(fovy, result.depth)));
   result.y = RvR_yres() / 2 - result.y + cam->shear;

   return result;
}

const RvR_ray_depth_buffer_entry *RvR_ray_depth_buffer_entry_floor(int x)
{
   return ray_depth_buffer.floor[x];
}

const RvR_ray_depth_buffer_entry *RvR_ray_depth_buffer_entry_ceiling(int x)
{
   return ray_depth_buffer.ceiling[x];
}

void RvR_ray_draw_map(const RvR_ray_cam *cam, const RvR_ray_map *map)
{
   RvR_fix16 dir0x = RvR_fix16_cos(cam->dir - (cam->fov / 2));
   RvR_fix16 dir0y = RvR_fix16_sin(cam->dir - (cam->fov / 2));
   RvR_fix16 dir1x = RvR_fix16_cos(cam->dir + (cam->fov / 2));
   RvR_fix16 dir1y = RvR_fix16_sin(cam->dir + (cam->fov / 2));
   RvR_fix16 cos = RvR_non_zero(RvR_fix16_cos(cam->fov / 2));
   dir0x = RvR_fix16_div(dir0x, cos);
   dir0y = RvR_fix16_div(dir0y, cos);
   dir1x = RvR_fix16_div(dir1x, cos);
   dir1y = RvR_fix16_div(dir1y, cos);

   RvR_fix16 dx = dir1x - dir0x;
   RvR_fix16 dy = dir1y - dir0y;
   RvR_fix16 current_dx = 0;
   RvR_fix16 current_dy = 0;

   RvR_ray_hit_result hits[RVR_RAY_MAX_STEPS] = {0};
   uint16_t hit_count = 0;

   for(int16_t x = 0; x<RvR_xres(); x++)
   {
      //Here by linearly interpolating the direction vector its length changes,
      //which in result achieves correcting the fish eye effect (computing
      //perpendicular distance).
      RvR_ray r;
      r.x = cam->x;
      r.y = cam->y;
      r.dirx = dir0x + (current_dx / (RvR_xres() - 1));
      r.diry = dir0y + (current_dy / (RvR_xres() - 1));
      current_dx += dx;
      current_dy += dy;

      hit_count = 0;

      RvR_fix16 current_posx = r.x;
      RvR_fix16 current_posy = r.y;
      int16_t current_squarex = (int16_t)(r.x / 65536);
      int16_t current_squarey = (int16_t)(r.y / 65536);

      RvR_fix16 old_floor = INT32_MIN;
      RvR_fix16 old_ceiling = INT32_MIN;
      uint16_t old_ftex = UINT16_MAX;
      uint16_t old_ctex = UINT16_MAX;
      RvR_fix16 floor = old_floor;
      RvR_fix16 ceiling = old_ceiling;
      uint16_t ftex = old_ftex;
      uint16_t ctex = old_ctex;

      RvR_fix16 deltax;
      if(RvR_abs(r.dirx)<=2) deltax = INT32_MAX; //Edge cases, would overflow otherwise
      else deltax = RvR_abs(RvR_fix16_div(65536, r.dirx));

      RvR_fix16 deltay;
      if(RvR_abs(r.diry)<=2) deltay = INT32_MAX; //Edge cases, would overflow otherwise
      else deltay = RvR_abs(RvR_fix16_div(65536, r.diry));

      int16_t stepx;
      int16_t stepy;
      RvR_fix16 side_distx;
      RvR_fix16 side_disty;
      int side = 0;

      if(r.dirx<0)
      {
         stepx = -1;
         side_distx = RvR_fix16_mul(r.x - current_squarex * 65536, deltax);
      }
      else
      {
         stepx = 1;
         side_distx = RvR_fix16_mul(current_squarex * 65536 + 65536 - r.x, deltax);
      }

      if(r.diry<0)
      {
         stepy = -1;
         side_disty = RvR_fix16_mul(r.y - current_squarey * 65536, deltay);
      }
      else
      {
         stepy = 1;
         side_disty = RvR_fix16_mul(current_squarey * 65536 + 65536 - r.y, deltay);
      }

      for(int i = 0; i<RVR_RAY_MAX_STEPS; i++)
      {
         // DDA step
         if(side_distx<side_disty)
         {
            side_distx += deltax;
            current_squarex += stepx;
            side = 0;
         }
         else
         {
            side_disty += deltay;
            current_squarey += stepy;
            side = 1;
         }

         int inbounds = RvR_ray_map_inbounds(map, current_squarex, current_squarey);
         if(inbounds)
         {
            floor = RvR_ray_map_floor_height_at_us(map, current_squarex, current_squarey);
            ceiling = RvR_ray_map_ceiling_height_at_us(map, current_squarex, current_squarey);
            ftex = RvR_ray_map_floor_tex_at_us(map, current_squarex, current_squarey);
            ctex = RvR_ray_map_ceil_tex_at_us(map, current_squarex, current_squarey);
         }

         if(!inbounds||i==RVR_RAY_MAX_STEPS - 1||floor!=old_floor||ceiling!=old_ceiling||ftex!=old_ftex||ctex!=old_ctex)
         {
            RvR_ray_hit_result h;
            h.posx = current_posx;
            h.posy = current_posy;
            h.squarex = current_squarex;
            h.squarey = current_squarey;

            if(!side)
            {
               h.distance = (side_distx - deltax);
               h.posy = r.y + RvR_fix16_mul(h.distance, r.diry);
               h.posx = current_squarex * 65536;
               h.direction = 3;
               if(stepx==-1)
               {
                  h.direction = 1;
                  h.posx += 65536;
               }
            }
            else
            {
               h.distance = (side_disty - deltay);
               h.posx = r.x + RvR_fix16_mul(h.distance, r.dirx);
               h.posy = current_squarey * 65536;
               h.direction = 2;
               if(stepy==-1)
               {
                  h.direction = 0;
                  h.posy += 65536;
               }
            }

            if(RvR_ray_map_inbounds(map, current_squarex, current_squarey))
            {
               h.wall_ftex = RvR_ray_map_wall_ftex_at_us(map, current_squarex, current_squarey);
               h.wall_ctex = RvR_ray_map_wall_ctex_at_us(map, current_squarex, current_squarey);
            }
            else
            {
               h.wall_ftex = map->sky_tex;
               h.wall_ctex = map->sky_tex;
            }

            h.fheight = 0;
            h.cheight = (127 * 65536) / 8;
            h.floor_tex = map->sky_tex;
            h.ceil_tex = map->sky_tex;

            switch(h.direction)
            {
            case 0:
               h.texture_coord = (h.posx) & 65535;
               if(RvR_ray_map_inbounds(map, current_squarex, current_squarey + 1))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map, current_squarex, current_squarey + 1);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map, current_squarex, current_squarey + 1);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map, h.squarex, h.squarey + 1);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map, h.squarex, h.squarey + 1);
               }
               break;
            case 1:
               h.texture_coord = (-h.posy) & 65535;
               if(RvR_ray_map_inbounds(map, current_squarex + 1, current_squarey))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map, current_squarex + 1, current_squarey);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map, current_squarex + 1, current_squarey);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map, h.squarex + 1, h.squarey);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map, h.squarex + 1, h.squarey);
               }
               break;
            case 2:
               h.texture_coord = (-h.posx) & 65535;
               if(RvR_ray_map_inbounds(map, current_squarex, current_squarey - 1))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map, current_squarex, current_squarey - 1);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map, current_squarex, current_squarey - 1);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map, h.squarex, h.squarey - 1);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map, h.squarex, h.squarey - 1);
               }
               break;
            case 3:
               h.texture_coord = (h.posy) & 65535;
               if(RvR_ray_map_inbounds(map, current_squarex - 1, current_squarey))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map, current_squarex - 1, current_squarey);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map, current_squarex - 1, current_squarey);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map, h.squarex - 1, h.squarey);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map, h.squarex - 1, h.squarey);
               }
               break;
            default:
               h.texture_coord = 0;
               break;
            }

            hits[hit_count++] = h;

            if(hit_count>=RVR_RAY_MAX_STEPS||!inbounds)
               break;

            old_floor = floor;
            old_ceiling = ceiling;
            old_ftex = ftex;
            old_ctex = ctex;
         }
      }

      ray_draw_column(map, cam, hits, hit_count, x, r);

   }

   //Render floor planes
   RvR_fix16 middle_row = (RvR_yres() / 2) + cam->shear;
   for(int i = 0; i<128; i++)
   {
      ray_plane *pl = ray_planes[i];
      while(pl!=NULL)
      {
         if(pl->min>pl->max)
         {
            pl = pl->next;
            continue;
         }

         //Sky texture is rendered differently (vertical collumns instead of horizontal ones)
         if(pl->tex==map->sky_tex)
         {
            RvR_texture *texture = RvR_texture_get(map->sky_tex);
            int skyw = 1 << RvR_log2(texture->width);
            int skyh = 1 << RvR_log2(texture->height);
            int mask = skyh - 1;

            RvR_fix16 angle_step = (skyw * 65536) / RvR_xres();
            RvR_fix16 tex_step = (65536 * skyh - 1) / RvR_yres();

            RvR_fix16 angle = (cam->dir) * 1024;
            angle += (pl->min - 1) * angle_step;

            for(int x = pl->min; x<pl->max + 1; x++)
            {
               //Sky is rendered fullbright, no lut needed
               uint8_t * restrict pix = &RvR_framebuffer()[(pl->start[x]) * RvR_xres() + x - 1];
               const uint8_t * restrict tex = &texture->data[((angle >> 16) & (skyw - 1)) * skyh];
               const uint8_t * restrict col = RvR_shade_table(32);

               //Split in two parts: above and below horizon
               int middle = RvR_max(0, RvR_min(RvR_yres(), middle_row + RvR_yres() / 32));
               int tex_start = pl->start[x];
               int tex_end = middle;
               if(tex_end>pl->end[x])
                  tex_end = pl->end[x];
               if(tex_start>tex_end)
                  tex_end = tex_start;
               if(tex_start>middle)
                  tex_end = tex_start - 1;
               int solid_end = pl->end[x];
               RvR_fix16 texture_coord = (RvR_yres() - middle + pl->start[x]) * tex_step;

               for(int y = tex_start; y<tex_end + 1; y++)
               {
                  *pix = tex[texture_coord >> 16];
                  texture_coord += tex_step;
                  pix += RvR_xres();
               }
               RvR_fix16 tex_coord = (RvR_yres()) * tex_step - 1;
               texture_coord = RvR_min(tex_coord, tex_coord - tex_step * (tex_end - middle));
               for(int y = tex_end + 1; y<solid_end + 1; y++)
               {
                  *pix = col[tex[(texture_coord >> 16) & mask]];
                  texture_coord -= tex_step;
                  pix += RvR_xres();
               }

               angle += angle_step;
            }

            pl = pl->next;
            continue;
         }

         //Convert plane to horizontal spans
         RvR_texture *texture = RvR_texture_get(pl->tex);
         for(int x = pl->min; x<pl->max + 2; x++)
         {
            RvR_fix16 s0 = pl->start[x - 1];
            RvR_fix16 s1 = pl->start[x];
            RvR_fix16 e0 = pl->end[x - 1];
            RvR_fix16 e1 = pl->end[x];

            //End spans top
            for(; s0<s1&&s0<=e0; s0++)
               ray_span_draw_tex(cam, ray_span_start[s0], x - 1, s0, pl->height, texture);

            //End spans bottom
            for(; e0>e1&&e0>=s0; e0--)
               ray_span_draw_tex(cam, ray_span_start[e0], x - 1, e0, pl->height, texture);

            //Start spans top
            for(; s1<s0&&s1<=e1; s1++)
               ray_span_start[s1] = x - 1;

            //Start spans bottom
            for(; e1>e0&&e1>=s1; e1--)
               ray_span_start[e1] = x - 1;
         }

         pl = pl->next;
      }
   }
   //-------------------------------------
}

static void ray_draw_column(const RvR_ray_map *map, const RvR_ray_cam *cam, RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray)
{
   //last written Y position, can never go backwards
   RvR_fix16 f_pos_y = RvR_yres();
   RvR_fix16 c_pos_y = -1;

   RvR_fix16 middle_row = RvR_yres() / 2 + cam->shear;
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov / 2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres() * fovx * 2, RvR_xres() << 16);

   //world coordinates (relative to camera height though)
   RvR_fix16 f_z1_world = RvR_ray_map_floor_height_at(map, (int16_t)(cam->x / 65536), (int16_t)(cam->y / 65536)) - cam->z;
   RvR_fix16 c_z1_world = RvR_ray_map_ceiling_height_at(map, (int16_t)(cam->x / 65536), (int16_t)(cam->y / 65536)) - cam->z;

   RvR_ray_pixel_info p = {0};
   RvR_ray_hit_result h = {0};
   p.x = x;

   //we'll be simulatenously drawing the floor and the ceiling now
   for(RvR_fix16 j = 0; j<=hits_len; j++)
   {                    //^ = add extra iteration for horizon plane
      int8_t drawing_horizon = j==(hits_len);
      int limit_c = 0;
      int limit_f = 0;

      RvR_fix16 distance = 0;

      RvR_fix16 f_z2_world = 0,    c_z2_world = 0;
      RvR_fix16 f_z1_screen = 0,   c_z1_screen = 0;
      RvR_fix16 f_z2_screen = 0,   c_z2_screen = 0;

      if(!drawing_horizon)
      {
         RvR_ray_hit_result hit = hits[j];
         distance = RvR_non_zero(hit.distance);
         h = hit;

         //Using RvR_fix16_div here messes up the rendering and I haven't quite
         //figured out why...
         //NOTE(Captain4LK): Might be because of i32 "overflow", since the value
         //calculated by RvR_fix16_div can't be presented as an int32
         RvR_fix16 wall_height = RvR_ray_map_floor_height_at(map, hit.squarex, hit.squarey);
         f_z2_world = wall_height - cam->z;
         f_z1_screen = middle_row - ((f_z1_world * RvR_yres()) / RvR_non_zero(RvR_fix16_mul(fovy, distance)));
         f_z2_screen = middle_row - ((f_z2_world * RvR_yres()) / RvR_non_zero(RvR_fix16_mul(fovy, distance)));

         wall_height = RvR_ray_map_ceiling_height_at(map, hit.squarex, hit.squarey);
         c_z2_world = wall_height - cam->z;
         c_z1_screen = middle_row - ((c_z1_world * RvR_yres()) / RvR_non_zero(RvR_fix16_mul(fovy, distance)));
         c_z2_screen = middle_row - ((c_z2_world * RvR_yres()) / RvR_non_zero(RvR_fix16_mul(fovy, distance)));
      }
      else
      {
         f_z1_screen = middle_row;
         c_z1_screen = middle_row + 1;

         h = hits[j - 1];
         h.distance = 65536 * 128;
         h.posx = RvR_fix16_mul(ray.dirx, h.distance);
         h.posy = RvR_fix16_mul(ray.diry, h.distance);

         h.direction = 0;
         h.texture_coord = 0;
         h.wall_ftex = map->sky_tex;
         h.wall_ctex = map->sky_tex;
         h.floor_tex = map->sky_tex;
         h.ceil_tex = map->sky_tex;
         h.fheight = 65536 * 128;
         h.cheight = 65536 * 128;
      }

      RvR_fix16 limit;

      //draw floor until wall
      limit_f = limit = RvR_clamp(f_z1_screen, c_pos_y + 1, RvR_yres());
      if(f_pos_y>limit)
      {
         ray_plane_add(h.fheight, h.floor_tex, p.x, limit, f_pos_y - 1);
         f_pos_y = limit;
      }

      //draw ceiling until wall
      limit_c = limit = RvR_clamp(c_z1_screen, -1, f_pos_y - 1);
      if(limit>c_pos_y)
      {
         ray_plane_add(h.cheight, h.ceil_tex, p.x, c_pos_y + 1, limit);
         c_pos_y = limit;
      }

      if(!drawing_horizon) //don't draw walls for horizon plane
      {
         p.depth = distance;
         p.depth = RvR_max(0, RvR_min(p.depth, (RVR_RAY_MAX_STEPS) * 65536));

         //draw floor wall
         if(f_z1_world!=f_z2_world)
         {
            if(f_pos_y>0)  //still pixels left?
            {
               limit = ray_draw_wall(map, cam, f_pos_y, f_z1_screen, f_z2_screen, c_pos_y + 1,
                                     RvR_yres(),
                                     //^ purposfully allow outside screen bounds here
                                     f_z2_world
                                     , -1, &p, &h);
               if(f_pos_y>limit)
                  f_pos_y = limit;

               f_z1_world = f_z2_world; //for the next iteration
               //^ purposfully allow outside screen bounds here
            }

            int limit_clip = RvR_min(limit, limit_f);
            RvR_ray_depth_buffer_entry *entry = ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip;
            entry->next = ray_depth_buffer.floor[p.x];
            ray_depth_buffer.floor[p.x] = entry;
         }

         //draw ceiling wall
         if(c_z1_world!=c_z2_world)
         {
            if(c_pos_y<RvR_yres() - 1) //pixels left?
            {
               limit = ray_draw_wall(map, cam, c_pos_y, c_z1_screen, c_z2_screen,
                                     -1, f_pos_y - 1,
                                     //^ puposfully allow outside screen bounds here
                                     c_z2_world
                                     , 1, &p, &h);

               if(c_pos_y<limit)
                  c_pos_y = limit;

               c_z1_world = c_z2_world; //for the next iteration
               //^ puposfully allow outside screen bounds here
            }

            int limit_clip = RvR_max(limit, limit_c);
            RvR_ray_depth_buffer_entry *entry = ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip + 1;
            entry->next = ray_depth_buffer.ceiling[p.x];
            ray_depth_buffer.ceiling[p.x] = entry;
         }
      }
   }
}

static int16_t ray_draw_wall(const RvR_ray_map *map, const RvR_ray_cam *cam, RvR_fix16 y_current, RvR_fix16 y_from, RvR_fix16 y_to, RvR_fix16 limit0, RvR_fix16 limit1, RvR_fix16 height, int16_t increment, RvR_ray_pixel_info *pixel_info, RvR_ray_hit_result *hit)
{
   int16_t limit = (int16_t)RvR_clamp(y_to, limit0, limit1);
   int start = 0;
   int end = 0;
   RvR_fix16 middle_row = RvR_yres() / 2 + cam->shear;
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov / 2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres() * fovx * 2, RvR_xres() * 65536);


   if(increment==-1)
   {
      start = limit;
      end = y_current + increment;
   }
   else if(increment==1)
   {
      start = y_current + increment;
      end = limit;
   }

   if(end<start)
      return limit;

   //Sky texture is handled differently and instead added as a plane
   if(increment==-1&&hit->wall_ftex==map->sky_tex)
   {
      ray_plane_add(hit->fheight, hit->wall_ftex, pixel_info->x, start, end);
      return limit;
   }
   else if(increment==1&&hit->wall_ctex==map->sky_tex)
   {
      ray_plane_add(hit->cheight, hit->wall_ctex, pixel_info->x, start, end);
      return limit;
   }

   RvR_texture *texture = NULL;
   //printf("wall %d\n",pixel_info->depth);
   RvR_fix16 coord_step_scaled = RvR_fix16_mul(64 * fovy, pixel_info->depth) / RvR_yres();
   RvR_fix16 texture_coord_scaled = 64 * height + (start - middle_row + 1) * coord_step_scaled;

   if(increment==-1)
      texture = RvR_texture_get(hit->wall_ftex);
   else if(increment==1)
      texture = RvR_texture_get(hit->wall_ctex);

   uint8_t * restrict pix = &RvR_framebuffer()[start * RvR_xres() + pixel_info->x];
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0, RvR_min(63, (hit->direction & 1) * 10 + (pixel_info->depth >> 15))));
   const uint8_t * restrict tex = &texture->data[(hit->texture_coord >> 10) * texture->height];
   RvR_fix16 y_and = (1 << RvR_log2(texture->height)) - 1;

#if RVR_UNROLL

   int count = end - start + 1;
   int n = (count + 7) / 8;
   switch(count % 8)
   {
   case 0: do {
         *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
         case 7: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
         case 6: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
         case 5: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
         case 4: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
         case 3: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
         case 2: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
         case 1: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RvR_xres(); //fallthrough
   }while(--n>0);
   }

#else

   for(int i = start; i<=end; i++)
   {
      *pix = col[tex[(texture_coord_scaled >> 16) & y_and]];
      texture_coord_scaled += coord_step_scaled;
      pix += RvR_xres();
   }

#endif

   return limit;
}

static void ray_span_draw_tex(const RvR_ray_cam *cam, int x0, int x1, int y, RvR_fix16 height, const RvR_texture *texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   if(texture==NULL)
      return;

   RvR_fix16 view_sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 view_cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov / 2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres() * fovx * 2, RvR_xres() << 16);
   RvR_fix16 middle_row = (RvR_yres() / 2) + cam->shear;

   RvR_fix16 dy = RvR_non_zero(middle_row - y); //Horizon is at infinity, use 1 above instead
   RvR_fix16 depth = RvR_fix16_div(RvR_abs(cam->z - height), RvR_non_zero(fovy));
   depth = RvR_fix16_div(depth * RvR_yres(), RvR_non_zero(RvR_abs(dy) << 16)); //TODO

   RvR_fix16 x_log = RvR_log2(texture->width);
   RvR_fix16 y_log = RvR_log2(texture->height);
   RvR_fix16 step_x = RvR_fix16_div(RvR_fix16_mul(view_sin, cam->z - height), RvR_non_zero(dy * 65536));
   RvR_fix16 step_y = RvR_fix16_div(RvR_fix16_mul(view_cos, cam->z - height), RvR_non_zero(dy * 65536));
   RvR_fix16 tx = cam->x + RvR_fix16_mul(view_cos, depth) + (x0 - RvR_xres() / 2) * step_x;
   RvR_fix16 ty = -cam->y - RvR_fix16_mul(view_sin, depth) + (x0 - RvR_xres() / 2) * step_y;
   RvR_fix16 x_and = (1 << x_log) - 1;
   RvR_fix16 y_and = (1 << y_log) - 1;

   uint8_t * restrict pix = RvR_framebuffer() + y * RvR_xres() + x0;
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0, RvR_min(63, (depth >> 15))));
   const uint8_t * restrict tex = texture->data;

   for(int x = x0; x<x1; x++)
   {
      uint8_t c = tex[(((tx >> 10) & x_and) << y_log) + ((ty >> 10) & y_and)];
      *pix = col[c];
      tx += step_x;
      ty += step_y;
      pix++;
   }
}

static void ray_plane_add(RvR_fix16 height, uint16_t tex, int x, int y0, int y1)
{
   x += 1;
   //Div height by 8192, since it's usually in these increments
   int hash = ((height / 8192) * 7 + tex * 3) & 127;

   ray_plane *pl = ray_planes[hash];
   while(pl!=NULL)
   {
      //ray_planes need to have the same height...
      if(height!=pl->height)
         goto next;
      //... and the same texture to be valid for concatination
      if(tex!=pl->tex)
         goto next;

      //Additionally the spans collumn needs to be either empty...
      if(pl->start[x]!=UINT16_MAX)
      {
         //... or directly adjacent to each other vertically, in that case
         //Concat planes vertically
         if(pl->start[x] - 1==(uint16_t)y1)
         {
            pl->start[x] = (uint16_t)y0;
            return;
         }
         if(pl->end[x] + 1==(uint16_t)y0)
         {
            pl->end[x] = (uint16_t)y1;
            return;
         }

         goto next;
      }

      break;
next:
      pl = pl->next;
   }

   if(pl==NULL)
   {
      pl = ray_plane_new();
      pl->next = ray_planes[hash];
      ray_planes[hash] = pl;

      pl->min = RvR_xres();
      pl->max = -1;
      pl->height = height;
      pl->tex = tex;

      //Since this is an unsigned int, we can use memset to set all values to 65535 (0xffff)
      memset(pl->start, 255, sizeof(pl->start));
   }

   if(x<pl->min)
      pl->min = x;
   if(x>pl->max)
      pl->max = x;

   pl->end[x] = (uint16_t)y1;
   pl->start[x] = (uint16_t)y0;
}

static RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_new()
{
   if(ray_depth_buffer_entry_pool==NULL)
   {
      RvR_ray_depth_buffer_entry *e = RvR_malloc(sizeof(*e) * 256, "RvR_ray ray_depth_buffer_entry pool");
      memset(e, 0, sizeof(*e) * 256);

      for(int i = 0; i<255; i++)
         e[i].next = &e[i + 1];
      ray_depth_buffer_entry_pool = e;
   }

   RvR_ray_depth_buffer_entry *e = ray_depth_buffer_entry_pool;
   ray_depth_buffer_entry_pool = e->next;
   e->next = NULL;

   return e;
}

static void ray_depth_buffer_entry_free(RvR_ray_depth_buffer_entry *ent)
{
   if(ent==NULL)
      return;

   //Find last
   RvR_ray_depth_buffer_entry *last = ent;
   while(last->next!=NULL)
      last = last->next;

   last->next = ray_depth_buffer_entry_pool;
   ray_depth_buffer_entry_pool = ent;
}

static ray_plane *ray_plane_new()
{
   if(ray_plane_pool==NULL)
   {
      ray_plane *p = RvR_malloc(sizeof(*p) * 16, "RvR_ray ray_plane pool");
      memset(p, 0, sizeof(*p) * 16);

      for(int i = 0; i<15; i++)
         p[i].next = &p[i + 1];
      ray_plane_pool = p;
   }

   ray_plane *p = ray_plane_pool;
   ray_plane_pool = p->next;
   p->next = NULL;

   return p;
}

static void ray_plane_free(ray_plane *pl)
{
   if(pl==NULL)
      return;

   //Find last
   ray_plane *last = pl;
   while(last->next!=NULL)
      last = last->next;

   last->next = ray_plane_pool;
   ray_plane_pool = pl;
}
//-------------------------------------
