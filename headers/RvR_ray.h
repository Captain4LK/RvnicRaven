#ifndef _RVR_RAY_H_

/*
   RvnicRaven - raycasting renderer

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

/*
   To create implementation (the function definitions) add
      #define RVR_RAY_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _RVR_RAY_H_

#ifndef RVR_RAY_MAX_STEPS
#define RVR_RAY_MAX_STEPS 64
#endif

typedef struct
{
   RvR_fix22_vec2 start;
   RvR_fix22_vec2 direction;
}RvR_ray;

typedef struct
{
   RvR_fix22_vec3 pos;
   RvR_fix22 direction;
   uint16_t texture;
   uint32_t flags;
   int32_t extra0;
   int32_t extra1;
   int32_t extra2;
}RvR_ray_map_sprite;

typedef struct
{
   uint16_t sky_tex;
   uint16_t width;
   uint16_t height;
   uint32_t sprite_count;
   RvR_fix22 *floor;
   RvR_fix22 *ceiling;
   uint16_t *floor_tex;
   uint16_t *ceil_tex;
   uint16_t *wall_ftex;
   uint16_t *wall_ctex;
   RvR_ray_map_sprite *sprites;
}RvR_ray_map;

typedef struct
{
   RvR_fix22_vec2 position;
   RvR_fix22_vec2 square;
   RvR_fix22 distance;
   uint8_t direction;
   uint16_t wall_ftex;
   uint16_t wall_ctex;
   uint16_t floor_tex;
   uint16_t ceil_tex;
   RvR_fix22 fheight;
   RvR_fix22 cheight;
   RvR_fix22 texture_coord;
}RvR_ray_hit_result;

typedef struct
{
   RvR_fix22_vec2 position;
   RvR_fix22 depth;
}RvR_ray_pixel_info;

typedef struct RvR_ray_depth_buffer_entry
{
   RvR_fix22 depth;
   int32_t limit;

   struct RvR_ray_depth_buffer_entry *next;
}RvR_ray_depth_buffer_entry;

typedef struct
{
   RvR_ray_depth_buffer_entry *floor[RVR_XRES];
   RvR_ray_depth_buffer_entry *ceiling[RVR_XRES];
}RvR_ray_depth_buffer;

typedef void (*RvR_ray_column_function) (RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray);

extern RvR_ray_map rvr_ray_map;

void RvR_ray_cast_multi_hit(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len);
void RvR_rays_cast_multi_hit(RvR_ray_column_function column);

void                  RvR_ray_cast_multi_hit_draw(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len);
void                  RvR_rays_cast_multi_hit_draw(RvR_ray_column_function column);
void                  RvR_ray_draw_begin();
void                  RvR_ray_draw_end();
//Only valid between RvR_ray_draw_begin() and RvR_ray_draw_end():
void                  RvR_ray_draw_map();
void                  RvR_ray_draw_sprite(RvR_fix22_vec3 pos, RvR_fix22 angle, uint16_t tex, uint32_t flags);
void                  RvR_ray_draw_debug(uint8_t index);
RvR_ray_depth_buffer *RvR_ray_draw_depth_buffer();
RvR_ray_pixel_info    RvR_ray_map_to_screen(RvR_fix22_vec3 world_position);

void      RvR_ray_set_angle(RvR_fix22 angle);
RvR_fix22 RvR_ray_get_angle();
void      RvR_ray_set_shear(int16_t shear);
int16_t   RvR_ray_get_shear();
void      RvR_ray_set_position(RvR_fix22_vec3 position);
RvR_fix22_vec3  RvR_ray_get_position();
void      RvR_ray_set_fov(RvR_fix22 fov);
RvR_fix22 RvR_ray_get_fov();

void RvR_ray_map_create(uint16_t width, uint16_t height);
void RvR_ray_map_free();
void RvR_ray_map_load(uint16_t id);
void RvR_ray_map_load_path(const char *path);
void RvR_ray_map_load_rw(RvR_rw *rw);
void RvR_ray_map_save(const char *path);

RvR_ray_map        *RvR_ray_map_get();
uint16_t            RvR_ray_map_sky_tex();
int                 RvR_ray_map_sprite_count();
RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index);

int       RvR_ray_map_inbounds(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_floor_height_at(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_ceiling_height_at(int16_t x, int16_t y);
uint16_t  RvR_ray_map_floor_tex_at(int16_t x, int16_t y);
uint16_t  RvR_ray_map_ceil_tex_at(int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ftex_at(int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ctex_at(int16_t x, int16_t y);

RvR_fix22 RvR_ray_map_floor_height_at_us(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_ceiling_height_at_us(int16_t x, int16_t y);
uint16_t  RvR_ray_map_floor_tex_at_us(int16_t x, int16_t y);
uint16_t  RvR_ray_map_ceil_tex_at_us(int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ftex_at_us(int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ctex_at_us(int16_t x, int16_t y);

void RvR_ray_map_floor_height_set(int16_t x, int16_t y, RvR_fix22 height);
void RvR_ray_map_ceiling_height_set(int16_t x, int16_t y, RvR_fix22 height);
void RvR_ray_map_floor_tex_set(int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_ceil_tex_set(int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_wall_ftex_set(int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_wall_ctex_set(int16_t x, int16_t y, uint16_t tex);

#endif

#ifdef RVR_RAY_IMPLEMENTATION
#ifndef RVR_RAY_IMPLEMENTATION_ONCE
#define RVR_RAY_IMPLEMENTATION_ONCE

#ifndef RVR_RAY_DRAW_PLANES
#define RVR_RAY_DRAW_PLANES 2
#endif

typedef struct rvr_ray_plane rvr_ray_plane;

struct rvr_ray_plane
{
   int32_t min;
   int32_t max;
   RvR_fix22 height;
   uint16_t tex;
   uint16_t start[RVR_XRES + 2];
   uint16_t end[RVR_XRES + 2];

   rvr_ray_plane *next;
};

typedef struct
{
   RvR_fix22_vec3 p;
   RvR_fix22_vec2 p0;
   RvR_fix22_vec2 p1;
   RvR_fix22_vec3 sp;
   RvR_fix22_vec3 sp0;
   RvR_fix22 st0;
   RvR_fix22_vec3 sp1;
   RvR_fix22 st1;
   uint16_t texture;
   uint32_t flags;
   RvR_fix22 angle;
}rvr_ray_sprite;

static RvR_fix22 rvr_ray_cam_angle = 0;
static RvR_fix22_vec3 rvr_ray_cam_position = {0};
static int16_t rvr_ray_cam_shear = 0;
static RvR_fix22 rvr_ray_cam_fov = 1024;
RvR_ray_map rvr_ray_map = {0};

static RvR_ray_depth_buffer rvr_ray_depth_buffer = {0};
static rvr_ray_plane *rvr_ray_planes[128] = {0};

//Linked list pools
static rvr_ray_plane *rvr_ray_plane_pool = NULL;
static RvR_ray_depth_buffer_entry *rvr_ray_depth_buffer_entry_pool = NULL;

static RvR_fix22 rvr_ray_start_floor_height = 0;
static RvR_fix22 rvr_ray_start_ceil_height = 0;
static int32_t rvr_ray_middle_row = 0;

struct
{
   rvr_ray_sprite * restrict data;
   uint_fast16_t * restrict data_proxy;
   uint32_t data_used;
   uint32_t data_size;
} rvr_ray_sprite_stack = {0};

static RvR_fix22 rvr_ray_fov_factor_x;
static RvR_fix22 rvr_ray_fov_factor_y;
static RvR_fix22 rvr_ray_sin;
static RvR_fix22 rvr_ray_cos;
static RvR_fix22 rvr_ray_sin_fov;
static RvR_fix22 rvr_ray_cos_fov;

static void rvr_ray_plane_add(RvR_fix22 height, uint16_t tex, int x, int y0, int y1);

static void rvr_ray_span_draw_tex(int x0, int x1, int y, RvR_fix22 height, const RvR_texture *texture);
static void rvr_ray_span_draw_flat(int x0, int x1, int y, uint8_t color);

static int16_t rvr_ray_draw_wall(RvR_fix22 y_current, RvR_fix22 y_from, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, RvR_fix22 height, int16_t increment, RvR_ray_pixel_info *pixel_info, RvR_ray_hit_result *hit);
static void rvr_ray_draw_column(RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray);

static void rvr_ray_sprite_stack_push(rvr_ray_sprite s);

static int rvr_ray_sprite_order(int a, int b);
static void rvr_ray_sprite_draw_wall(rvr_ray_sprite *sp);
static void rvr_ray_sprite_draw_floor(rvr_ray_sprite *sp);
static void rvr_ray_sprite_draw_billboard(rvr_ray_sprite *sp);

static RvR_ray_depth_buffer_entry *rvr_ray_depth_buffer_entry_new();
static void rvr_ray_depth_buffer_entry_free(RvR_ray_depth_buffer_entry *ent);

static rvr_ray_plane *rvr_ray_plane_new();
static void rvr_ray_plane_free(rvr_ray_plane *pl);

void RvR_ray_cast_multi_hit(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len)
{
   *hit_results_len = 0;

   RvR_fix22_vec2 current_pos = ray.start;
   RvR_fix22_vec2 current_square;

   current_square.x = ray.start.x / 1024;
   current_square.y = ray.start.y / 1024;

   RvR_fix22_vec2 delta;
   delta.x = RvR_abs((1024 * 1024) / RvR_non_zero(ray.direction.x));
   delta.y = RvR_abs((1024 * 1024) / RvR_non_zero(ray.direction.y));

   RvR_fix22_vec2 step;
   RvR_fix22_vec2 side_dist;
   int side;

   if(ray.direction.x<0)
   {
      step.x = -1;
      side_dist.x = ((ray.start.x - current_square.x * 1024) * delta.x) / 1024;
   }
   else
   {
      step.x = 1;
      side_dist.x = ((current_square.x * 1024 + 1024 - ray.start.x) * delta.x) / 1024;
   }

   if(ray.direction.y<0)
   {
      step.y = -1;
      side_dist.y = ((ray.start.y - current_square.y * 1024) * delta.y) / 1024;
   }
   else
   {
      step.y = 1;
      side_dist.y = ((current_square.y * 1024 + 1024 - ray.start.y) * delta.y) / 1024;
   }

   for(unsigned i = 0; i<RVR_RAY_MAX_STEPS; i++)
   {
      // DDA step
      if(side_dist.x<side_dist.y)
      {
         side_dist.x += delta.x;
         current_square.x += step.x;
         side = 0;
      }
      else
      {
         side_dist.y += delta.y;
         current_square.y += step.y;
         side = 1;
      }

      RvR_ray_hit_result h;
      h.position = current_pos;
      h.square   = current_square;

      if(!side)
      {
         h.distance = (side_dist.x - delta.x);
         h.position.y = ray.start.y + (h.distance * ray.direction.y) / 1024;
         h.position.x = current_square.x * 1024;
         h.direction = 3;
         if(step.x==-1)
         {
            h.direction = 1;
            h.position.x += 1024;
         }
      }
      else
      {
         h.distance = (side_dist.y - delta.y);
         h.position.x = ray.start.x + (h.distance * ray.direction.x) / 1024;
         h.position.y = current_square.y * 1024;
         h.direction = 2;
         if(step.y==-1)
         {
            h.direction = 0;
            h.position.y += 1024;
         }
      }

      if(RvR_ray_map_inbounds(current_square.x, current_square.y))
      {
         h.wall_ftex = RvR_ray_map_wall_ftex_at_us(current_square.x, current_square.y);
         h.wall_ctex = RvR_ray_map_wall_ctex_at_us(current_square.x, current_square.y);
      }
      else
      {
         h.wall_ftex = RvR_ray_map_sky_tex();
         h.wall_ctex = RvR_ray_map_sky_tex();
      }

      h.fheight = 0;
      h.cheight = (127 * 1024) / 8;
      h.floor_tex = RvR_ray_map_sky_tex();
      h.ceil_tex = RvR_ray_map_sky_tex();

      switch(h.direction)
      {
      case 0:
         h.texture_coord = (h.position.x) & 1023;
         if(RvR_ray_map_inbounds(current_square.x, current_square.y + 1))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x, current_square.y + 1);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x, current_square.y + 1);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x, h.square.y + 1);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x, h.square.y + 1);
         }
         break;
      case 1:
         h.texture_coord = (-h.position.y) & 1023;
         if(RvR_ray_map_inbounds(current_square.x + 1, current_square.y))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x + 1, current_square.y);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x + 1, current_square.y);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x + 1, h.square.y);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x + 1, h.square.y);
         }
         break;
      case 2:
         h.texture_coord = (-h.position.x) & 1023;
         if(RvR_ray_map_inbounds(current_square.x, current_square.y - 1))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x, current_square.y - 1);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x, current_square.y - 1);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x, h.square.y - 1);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x, h.square.y - 1);
         }
         break;
      case 3:
         h.texture_coord = (h.position.y) & 1023;
         if(RvR_ray_map_inbounds(current_square.x - 1, current_square.y))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x - 1, current_square.y);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x - 1, current_square.y);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x - 1, h.square.y);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x - 1, h.square.y);
         }
         break;
      default:
         h.texture_coord = 0;
         break;
      }

      hit_results[*hit_results_len] = h;
      (*hit_results_len)++;

      if(*hit_results_len>=RVR_RAY_MAX_STEPS)
         break;
   }
}

void RvR_rays_cast_multi_hit(RvR_ray_column_function column)
{
   RvR_fix22_vec2 dir0 = RvR_fix22_vec2_rot(rvr_ray_cam_angle - (rvr_ray_cam_fov / 2));
   RvR_fix22_vec2 dir1 = RvR_fix22_vec2_rot(rvr_ray_cam_angle + (rvr_ray_cam_fov / 2));
   RvR_fix22 cos = RvR_non_zero(RvR_fix22_cos(rvr_ray_cam_fov / 2));
   dir0.x = (dir0.x * 1024) / cos;
   dir0.y = (dir0.y * 1024) / cos;
   dir1.x = (dir1.x * 1024) / cos;
   dir1.y = (dir1.y * 1024) / cos;

   RvR_fix22 dx = dir1.x - dir0.x;
   RvR_fix22 dy = dir1.y - dir0.y;
   RvR_fix22 current_dx = 0;
   RvR_fix22 current_dy = 0;

   RvR_ray_hit_result hits[RVR_RAY_MAX_STEPS] = {0};
   uint16_t hit_count = 0;

   for(int16_t i = 0; i<RVR_XRES; i++)
   {
      //Here by linearly interpolating the direction vector its length changes,
      //which in result achieves correcting the fish eye effect (computing
      //perpendicular distance).
      RvR_ray r;
      r.start = (RvR_fix22_vec2) {
         rvr_ray_cam_position.x, rvr_ray_cam_position.y
      };
      r.direction.x = dir0.x + (current_dx / RVR_XRES - 1);
      r.direction.y = dir0.y + (current_dy / RVR_XRES - 1);

      RvR_ray_cast_multi_hit(r, hits, &hit_count);

      column(hits, hit_count, i, r);

      current_dx += dx;
      current_dy += dy;
   }
}

void RvR_ray_cast_multi_hit_draw(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len)
{
   *hit_results_len = 0;

   RvR_fix22_vec2 current_pos = ray.start;
   RvR_fix22_vec2 current_square;

   current_square.x = ray.start.x / 1024;
   current_square.y = ray.start.y / 1024;

   RvR_fix22 old_floor = INT32_MIN;
   RvR_fix22 old_ceiling = INT32_MIN;
   uint16_t old_ftex = UINT16_MAX;
   uint16_t old_ctex = UINT16_MAX;
   RvR_fix22 floor = old_floor;
   RvR_fix22 ceiling = old_ceiling;
   uint16_t ftex = old_ftex;
   uint16_t ctex = old_ctex;

   RvR_fix22_vec2 delta;
   delta.x = RvR_abs((1024 * 1024) / RvR_non_zero(ray.direction.x));
   delta.y = RvR_abs((1024 * 1024) / RvR_non_zero(ray.direction.y));

   RvR_fix22_vec2 step;
   RvR_fix22_vec2 side_dist;
   int side;

   if(ray.direction.x<0)
   {
      step.x = -1;
      side_dist.x = ((ray.start.x - current_square.x * 1024) * delta.x) / 1024;
   }
   else
   {
      step.x = 1;
      side_dist.x = ((current_square.x * 1024 + 1024 - ray.start.x) * delta.x) / 1024;
   }

   if(ray.direction.y<0)
   {
      step.y = -1;
      side_dist.y = ((ray.start.y - current_square.y * 1024) * delta.y) / 1024;
   }
   else
   {
      step.y = 1;
      side_dist.y = ((current_square.y * 1024 + 1024 - ray.start.y) * delta.y) / 1024;
   }

   for(unsigned i = 0; i<RVR_RAY_MAX_STEPS; i++)
   {
      // DDA step
      if(side_dist.x<side_dist.y)
      {
         side_dist.x += delta.x;
         current_square.x += step.x;
         side = 0;
      }
      else
      {
         side_dist.y += delta.y;
         current_square.y += step.y;
         side = 1;
      }

      int inbounds = RvR_ray_map_inbounds(current_square.x, current_square.y);
      if(inbounds)
      {
         floor = RvR_ray_map_floor_height_at_us(current_square.x, current_square.y);
         ceiling = RvR_ray_map_ceiling_height_at_us(current_square.x, current_square.y);
         ftex = RvR_ray_map_floor_tex_at_us(current_square.x, current_square.y);
         ctex = RvR_ray_map_ceil_tex_at_us(current_square.x, current_square.y);
      }

      if(!inbounds||i==RVR_RAY_MAX_STEPS - 1||floor!=old_floor||ceiling!=old_ceiling||ftex!=old_ftex||ctex!=old_ctex)
      {
         RvR_ray_hit_result h;
         h.position = current_pos;
         h.square   = current_square;

         if(!side)
         {
            h.distance = (side_dist.x - delta.x);
            h.position.y = ray.start.y + (h.distance * ray.direction.y) / 1024;
            h.position.x = current_square.x * 1024;
            h.direction = 3;
            if(step.x==-1)
            {
               h.direction = 1;
               h.position.x += 1024;
            }
         }
         else
         {
            h.distance = (side_dist.y - delta.y);
            h.position.x = ray.start.x + (h.distance * ray.direction.x) / 1024;
            h.position.y = current_square.y * 1024;
            h.direction = 2;
            if(step.y==-1)
            {
               h.direction = 0;
               h.position.y += 1024;
            }
         }

         if(RvR_ray_map_inbounds(current_square.x, current_square.y))
         {
            h.wall_ftex = RvR_ray_map_wall_ftex_at_us(current_square.x, current_square.y);
            h.wall_ctex = RvR_ray_map_wall_ctex_at_us(current_square.x, current_square.y);
         }
         else
         {
            h.wall_ftex = RvR_ray_map_sky_tex();
            h.wall_ctex = RvR_ray_map_sky_tex();
         }

         h.fheight = 0;
         h.cheight = (127 * 1024) / 8;
         h.floor_tex = RvR_ray_map_sky_tex();
         h.ceil_tex = RvR_ray_map_sky_tex();

         switch(h.direction)
         {
         case 0:
            h.texture_coord = (h.position.x) & 1023;
            if(RvR_ray_map_inbounds(current_square.x, current_square.y + 1))
            {
               h.fheight = RvR_ray_map_floor_height_at_us(current_square.x, current_square.y + 1);
               h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x, current_square.y + 1);
               h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x, h.square.y + 1);
               h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x, h.square.y + 1);
            }
            break;
         case 1:
            h.texture_coord = (-h.position.y) & 1023;
            if(RvR_ray_map_inbounds(current_square.x + 1, current_square.y))
            {
               h.fheight = RvR_ray_map_floor_height_at_us(current_square.x + 1, current_square.y);
               h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x + 1, current_square.y);
               h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x + 1, h.square.y);
               h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x + 1, h.square.y);
            }
            break;
         case 2:
            h.texture_coord = (-h.position.x) & 1023;
            if(RvR_ray_map_inbounds(current_square.x, current_square.y - 1))
            {
               h.fheight = RvR_ray_map_floor_height_at_us(current_square.x, current_square.y - 1);
               h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x, current_square.y - 1);
               h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x, h.square.y - 1);
               h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x, h.square.y - 1);
            }
            break;
         case 3:
            h.texture_coord = (h.position.y) & 1023;
            if(RvR_ray_map_inbounds(current_square.x - 1, current_square.y))
            {
               h.fheight = RvR_ray_map_floor_height_at_us(current_square.x - 1, current_square.y);
               h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x - 1, current_square.y);
               h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x - 1, h.square.y);
               h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x - 1, h.square.y);
            }
            break;
         default:
            h.texture_coord = 0;
            break;
         }

         hit_results[*hit_results_len] = h;
         (*hit_results_len)++;

         if(*hit_results_len>=RVR_RAY_MAX_STEPS||!inbounds)
            break;

         old_floor = floor;
         old_ceiling = ceiling;
         old_ftex = ftex;
         old_ctex = ctex;
      }
   }
}

void RvR_rays_cast_multi_hit_draw(RvR_ray_column_function column)
{
   RvR_fix22_vec2 dir0 = RvR_fix22_vec2_rot(rvr_ray_cam_angle - (rvr_ray_cam_fov / 2));
   RvR_fix22_vec2 dir1 = RvR_fix22_vec2_rot(rvr_ray_cam_angle + (rvr_ray_cam_fov / 2));
   RvR_fix22 cos = RvR_non_zero(RvR_fix22_cos(rvr_ray_cam_fov / 2));
   dir0.x = (dir0.x * 1024) / cos;
   dir0.y = (dir0.y * 1024) / cos;
   dir1.x = (dir1.x * 1024) / cos;
   dir1.y = (dir1.y * 1024) / cos;

   RvR_fix22 dx = dir1.x - dir0.x;
   RvR_fix22 dy = dir1.y - dir0.y;
   RvR_fix22 current_dx = 0;
   RvR_fix22 current_dy = 0;

   RvR_ray_hit_result hits[RVR_RAY_MAX_STEPS] = {0};
   uint16_t hit_count = 0;

   for(int16_t i = 0; i<RVR_XRES; i++)
   {
      //Here by linearly interpolating the direction vector its length changes,
      //which in result achieves correcting the fish eye effect (computing
      //perpendicular distance).
      RvR_ray r;
      r.start = (RvR_fix22_vec2) {
         rvr_ray_cam_position.x, rvr_ray_cam_position.y
      };
      r.direction.x = dir0.x + (current_dx / (RVR_XRES - 1));
      r.direction.y = dir0.y + (current_dy / (RVR_XRES - 1));

      RvR_ray_cast_multi_hit_draw(r, hits, &hit_count);

      column(hits, hit_count, i, r);

      current_dx += dx;
      current_dy += dy;
   }
}

void RvR_ray_set_angle(RvR_fix22 angle)
{
   rvr_ray_cam_angle = angle;
}

RvR_fix22 RvR_ray_get_angle()
{
   return rvr_ray_cam_angle;
}

void RvR_ray_set_shear(int16_t shear)
{
   rvr_ray_cam_shear = shear;
}

int16_t RvR_ray_get_shear()
{
   return rvr_ray_cam_shear;
}

void RvR_ray_set_position(RvR_fix22_vec3 position)
{
   rvr_ray_cam_position = position;
}

RvR_fix22_vec3 RvR_ray_get_position()
{
   return rvr_ray_cam_position;
}

void RvR_ray_set_fov(RvR_fix22 fov)
{
   rvr_ray_cam_fov = fov;
}

RvR_fix22 RvR_ray_get_fov()
{
   return rvr_ray_cam_fov;
}

void RvR_ray_map_create(uint16_t width, uint16_t height)
{
   if(width==0)
      width = 1;
   if(height==0)
      height = 1;

   RvR_ray_map_free();

   rvr_ray_map.width = width;
   rvr_ray_map.height = height;
   rvr_ray_map.sky_tex = 0;
   rvr_ray_map.sprite_count = 0;
   rvr_ray_map.floor = RvR_malloc(rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.floor));
   rvr_ray_map.ceiling = RvR_malloc(rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.ceiling));
   rvr_ray_map.floor_tex = RvR_malloc(rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.floor_tex));
   rvr_ray_map.ceil_tex = RvR_malloc(rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.ceil_tex));
   rvr_ray_map.wall_ftex = RvR_malloc(rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.wall_ftex));
   rvr_ray_map.wall_ctex = RvR_malloc(rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.wall_ctex));

   memset(rvr_ray_map.floor, 0, rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.floor));
   memset(rvr_ray_map.floor_tex, 0, rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.floor_tex));
   memset(rvr_ray_map.ceil_tex, 0, rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.ceil_tex));
   memset(rvr_ray_map.wall_ftex, 0, rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.wall_ftex));
   memset(rvr_ray_map.wall_ctex, 0, rvr_ray_map.width * rvr_ray_map.height * sizeof(*rvr_ray_map.wall_ctex));

   for(int y = 0; y<rvr_ray_map.height; y++)
      for(int x = 0; x<rvr_ray_map.width; x++)
         rvr_ray_map.ceiling[y * rvr_ray_map.width + x] = 32 * 128;
}

void RvR_ray_map_free()
{
   if(rvr_ray_map.floor!=NULL)
      RvR_free(rvr_ray_map.floor);
   if(rvr_ray_map.ceiling!=NULL)
      RvR_free(rvr_ray_map.ceiling);
   if(rvr_ray_map.floor_tex!=NULL)
      RvR_free(rvr_ray_map.floor_tex);
   if(rvr_ray_map.ceil_tex!=NULL)
      RvR_free(rvr_ray_map.ceil_tex);
   if(rvr_ray_map.wall_ftex!=NULL)
      RvR_free(rvr_ray_map.wall_ftex);
   if(rvr_ray_map.wall_ctex!=NULL)
      RvR_free(rvr_ray_map.wall_ctex);
   if(rvr_ray_map.sprites!=NULL)
      RvR_free(rvr_ray_map.sprites);

   rvr_ray_map.floor = NULL;
   rvr_ray_map.floor_tex = NULL;
   rvr_ray_map.ceil_tex = NULL;
   rvr_ray_map.ceiling = NULL;
   rvr_ray_map.wall_ftex = NULL;
   rvr_ray_map.wall_ctex = NULL;
   rvr_ray_map.sprites = NULL;
   rvr_ray_map.width = 0;
   rvr_ray_map.height = 0;
   rvr_ray_map.sprite_count = 0;
}

void RvR_ray_map_load(uint16_t id)
{
   char tmp[64];
   sprintf(tmp, "MAP%05d", id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   mem_pak = RvR_lump_get(tmp, &size_in);
   RvR_rw rw_decomp;
   RvR_rw_init_const_mem(&rw_decomp, mem_pak, size_in);
   mem_decomp = RvR_decompress(&rw_decomp, &size_out);
   RvR_rw_close(&rw_decomp);
   RvR_free(mem_pak);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw, mem_decomp, size_out);
   RvR_ray_map_load_rw(&rw);
   RvR_rw_close(&rw);

   RvR_free(mem_decomp);
}

void RvR_ray_map_load_path(const char *path)
{
   RvR_error_check(path!=NULL, "RvR_ray_map_load_path", "argument 'path' must be non-NULL\n");

   int32_t size = 0;
   RvR_rw rw_decomp;
   RvR_rw_init_path(&rw_decomp, path, "rb");
   uint8_t *mem = RvR_decompress(&rw_decomp, &size);
   RvR_rw_close(&rw_decomp);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw, mem, size);
   RvR_ray_map_load_rw(&rw);
   RvR_rw_close(&rw);

   RvR_free(mem);

RvR_err:
   return;
}

void RvR_ray_map_load_rw(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL, "RvR_ray_map_load_rw", "argument 'rw' must be non-NULL\n");

   //Read and check version
   uint16_t version = RvR_rw_read_u16(rw);
   RvR_error_check(version==0, "RvR_ray_map_load_rw", "Invalid version '%d', expected version '0'\n", version);

   //Read sky texture
   uint16_t sky_tex = RvR_rw_read_u16(rw);

   //Read level width and height
   uint16_t width = RvR_rw_read_u16(rw);
   uint16_t height = RvR_rw_read_u16(rw);

   //Create map
   RvR_ray_map_create(width, height);
   rvr_ray_map.sky_tex = sky_tex;

   //Read sprite count
   rvr_ray_map.sprite_count = RvR_rw_read_u32(rw);
   rvr_ray_map.sprites = RvR_malloc(sizeof(*rvr_ray_map.sprites) * rvr_ray_map.sprite_count);

   //Read texture, floor and ceiling
   int32_t tile_count = rvr_ray_map.width * rvr_ray_map.height;
   for(int32_t i = 0; i<tile_count; i++) rvr_ray_map.floor[i] = RvR_rw_read_u32(rw);
   for(int32_t i = 0; i<tile_count; i++) rvr_ray_map.ceiling[i] = RvR_rw_read_u32(rw);
   for(int32_t i = 0; i<tile_count; i++) rvr_ray_map.floor_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0; i<tile_count; i++) rvr_ray_map.ceil_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0; i<tile_count; i++) rvr_ray_map.wall_ftex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0; i<tile_count; i++) rvr_ray_map.wall_ctex[i] = RvR_rw_read_u16(rw);

   //Read sprites
   for(unsigned i = 0; i<rvr_ray_map.sprite_count; i++)
   {
      rvr_ray_map.sprites[i].pos.x = RvR_rw_read_u32(rw);
      rvr_ray_map.sprites[i].pos.y = RvR_rw_read_u32(rw);
      rvr_ray_map.sprites[i].pos.z = RvR_rw_read_u32(rw);
      rvr_ray_map.sprites[i].direction = RvR_rw_read_u32(rw);
      rvr_ray_map.sprites[i].texture = RvR_rw_read_u16(rw);
      rvr_ray_map.sprites[i].flags = RvR_rw_read_u32(rw);
      rvr_ray_map.sprites[i].extra0 = RvR_rw_read_u32(rw);
      rvr_ray_map.sprites[i].extra1 = RvR_rw_read_u32(rw);
      rvr_ray_map.sprites[i].extra2 = RvR_rw_read_u32(rw);
   }

RvR_err:
   return;
}

void RvR_ray_map_save(const char *path)
{
   RvR_error_check(path!=NULL, "RvR_ray_map_save", "argument 'path' must be non-NULL\n");

   //Calculate needed memory
   int size = 0;
   size += 2; //version
   size += 2; //rvr_ray_map.sky_tex
   size += 2; //rvr_ray_map.width
   size += 2; //rvr_ray_map.height
   size += 4; //rvr_ray_map.sprite_count
   size += rvr_ray_map.width * rvr_ray_map.height * 4; //rvr_ray_map.floor
   size += rvr_ray_map.width * rvr_ray_map.height * 4; //rvr_ray_map.ceiling
   size += rvr_ray_map.width * rvr_ray_map.height * 2; //rvr_ray_map.floor_tex
   size += rvr_ray_map.width * rvr_ray_map.height * 2; //rvr_ray_map.ceil_tex
   size += rvr_ray_map.width * rvr_ray_map.height * 2; //rvr_ray_map.wall_ftex
   size += rvr_ray_map.width * rvr_ray_map.height * 2; //rvr_ray_map.wall_ctex
   size += rvr_ray_map.sprite_count * sizeof(*rvr_ray_map.sprites);

   uint8_t *mem = RvR_malloc(size);
   RvR_rw rw = {0};
   RvR_rw_init_mem(&rw, mem, size, size);

   //version
   RvR_rw_write_u16(&rw, 0);

   //sky texture
   RvR_rw_write_u16(&rw, rvr_ray_map.sky_tex);

   //width and height
   RvR_rw_write_u16(&rw, rvr_ray_map.width);
   RvR_rw_write_u16(&rw, rvr_ray_map.height);

   //sprite count
   RvR_rw_write_u32(&rw, rvr_ray_map.sprite_count);

   //floor height
   for(int i = 0; i<rvr_ray_map.width * rvr_ray_map.height; i++)
      RvR_rw_write_u32(&rw, rvr_ray_map.floor[i]);

   //ceiling height
   for(int i = 0; i<rvr_ray_map.width * rvr_ray_map.height; i++)
      RvR_rw_write_u32(&rw, rvr_ray_map.ceiling[i]);

   //floor texture
   for(int i = 0; i<rvr_ray_map.width * rvr_ray_map.height; i++)
      RvR_rw_write_u16(&rw, rvr_ray_map.floor_tex[i]);

   //ceiling texture
   for(int i = 0; i<rvr_ray_map.width * rvr_ray_map.height; i++)
      RvR_rw_write_u16(&rw, rvr_ray_map.ceil_tex[i]);

   //wall ftex
   for(int i = 0; i<rvr_ray_map.width * rvr_ray_map.height; i++)
      RvR_rw_write_u16(&rw, rvr_ray_map.wall_ftex[i]);

   //wall ctex
   for(int i = 0; i<rvr_ray_map.width * rvr_ray_map.height; i++)
      RvR_rw_write_u16(&rw, rvr_ray_map.wall_ctex[i]);

   //sprites
   for(unsigned i = 0; i<rvr_ray_map.sprite_count; i++)
   {
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].pos.x);
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].pos.y);
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].pos.z);
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].direction);
      RvR_rw_write_u16(&rw, rvr_ray_map.sprites[i].texture);
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].flags);
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].extra0);
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].extra1);
      RvR_rw_write_u32(&rw, rvr_ray_map.sprites[i].extra2);
   }

   //Compress and write to disk
   RvR_rw in;
   RvR_rw out;
   RvR_rw_init_mem(&in, mem, size, size);
   RvR_rw_init_path(&out, path, "wb");
   RvR_compress(&in, &out, 0);
   RvR_rw_close(&in);
   RvR_rw_close(&out);

   //Free temp buffer
   RvR_rw_close(&rw);
   RvR_free(mem);

RvR_err:
   return;
}

RvR_ray_map *RvR_ray_map_get()
{
   return &rvr_ray_map;
}

uint16_t RvR_ray_map_sky_tex()
{
   return rvr_ray_map.sky_tex;
}

int RvR_ray_map_sprite_count()
{
   return rvr_ray_map.sprite_count;
}

RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index)
{
   if(rvr_ray_map.sprites==NULL||index>=rvr_ray_map.sprite_count)
      return NULL;
   return &rvr_ray_map.sprites[index];
}

int RvR_ray_map_inbounds(int16_t x, int16_t y)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      return 1;
   return 0;
}

RvR_fix22 RvR_ray_map_floor_height_at(int16_t x, int16_t y)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      return rvr_ray_map.floor[y * rvr_ray_map.width + x];

   return 127 * 128;
}

RvR_fix22 RvR_ray_map_ceiling_height_at(int16_t x, int16_t y)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      return rvr_ray_map.ceiling[y * rvr_ray_map.width + x];

   return 127 * 128;
}

uint16_t RvR_ray_map_floor_tex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      return rvr_ray_map.floor_tex[y * rvr_ray_map.width + x];

   return 0;
}

uint16_t RvR_ray_map_ceil_tex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      return rvr_ray_map.ceil_tex[y * rvr_ray_map.width + x];

   return rvr_ray_map.sky_tex;
}

uint16_t RvR_ray_map_wall_ftex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      return rvr_ray_map.wall_ftex[y * rvr_ray_map.width + x];

   return 0;
}

uint16_t RvR_ray_map_wall_ctex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      return rvr_ray_map.wall_ctex[y * rvr_ray_map.width + x];

   return 0;
}

RvR_fix22 RvR_ray_map_floor_height_at_us(int16_t x, int16_t y)
{
   return rvr_ray_map.floor[y * rvr_ray_map.width + x];
}

RvR_fix22 RvR_ray_map_ceiling_height_at_us(int16_t x, int16_t y)
{
   return rvr_ray_map.ceiling[(y) * rvr_ray_map.width + x];
}

uint16_t RvR_ray_map_floor_tex_at_us(int16_t x, int16_t y)
{
   return rvr_ray_map.floor_tex[y * rvr_ray_map.width + x];
}

uint16_t RvR_ray_map_ceil_tex_at_us(int16_t x, int16_t y)
{
   return rvr_ray_map.ceil_tex[y * rvr_ray_map.width + x];
}

uint16_t RvR_ray_map_wall_ftex_at_us(int16_t x, int16_t y)
{
   return rvr_ray_map.wall_ftex[y * rvr_ray_map.width + x];
}

uint16_t RvR_ray_map_wall_ctex_at_us(int16_t x, int16_t y)
{
   return rvr_ray_map.wall_ctex[y * rvr_ray_map.width + x];
}

void RvR_ray_map_floor_height_set(int16_t x, int16_t y, RvR_fix22 height)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      rvr_ray_map.floor[y * rvr_ray_map.width + x] = height;
}

void RvR_ray_map_ceiling_height_set(int16_t x, int16_t y, RvR_fix22 height)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      rvr_ray_map.ceiling[y * rvr_ray_map.width + x] = height;
}

void RvR_ray_map_floor_tex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      rvr_ray_map.floor_tex[y * rvr_ray_map.width + x] = tex;
}

void RvR_ray_map_ceil_tex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      rvr_ray_map.ceil_tex[y * rvr_ray_map.width + x] = tex;
}

void RvR_ray_map_wall_ftex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      rvr_ray_map.wall_ftex[y * rvr_ray_map.width + x] = tex;
}

void RvR_ray_map_wall_ctex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<rvr_ray_map.width&&y>=0&&y<rvr_ray_map.height)
      rvr_ray_map.wall_ctex[y * rvr_ray_map.width + x] = tex;
}

void RvR_ray_draw_begin()
{
   //Clear depth buffer
   for(int i = 0; i<RVR_XRES; i++)
   {
      rvr_ray_depth_buffer_entry_free(rvr_ray_depth_buffer.floor[i]);
      rvr_ray_depth_buffer_entry_free(rvr_ray_depth_buffer.ceiling[i]);

      rvr_ray_depth_buffer.floor[i] = NULL;
      rvr_ray_depth_buffer.ceiling[i] = NULL;
   }

   //Clear planes
   for(int i = 0; i<128; i++)
   {
      rvr_ray_plane_free(rvr_ray_planes[i]);
      rvr_ray_planes[i] = NULL;
   }

   //Initialize needed vars
   rvr_ray_fov_factor_x = RvR_fix22_tan(RvR_ray_get_fov() / 2);
   rvr_ray_fov_factor_y = (RVR_YRES * rvr_ray_fov_factor_x * 2) / RVR_XRES;

   rvr_ray_middle_row = (RVR_YRES / 2) + RvR_ray_get_shear();

   rvr_ray_start_floor_height = RvR_ray_map_floor_height_at(RvR_ray_get_position().x / 1024, RvR_ray_get_position().y / 1024) - RvR_ray_get_position().z;
   rvr_ray_start_ceil_height = RvR_ray_map_ceiling_height_at(RvR_ray_get_position().x / 1024, RvR_ray_get_position().y / 1024) - RvR_ray_get_position().z;

   rvr_ray_cos = RvR_fix22_cos(RvR_ray_get_angle());
   rvr_ray_sin = RvR_fix22_sin(RvR_ray_get_angle());
   rvr_ray_cos_fov = (rvr_ray_cos * rvr_ray_fov_factor_x) / 1024;
   rvr_ray_sin_fov = (rvr_ray_sin * rvr_ray_fov_factor_x) / 1024;
}

void RvR_ray_draw_end()
{
   while(rvr_ray_sprite_stack.data_used>0)
   {
      uint8_t certain[rvr_ray_sprite_stack.data_used];
      memset(certain, 0, sizeof(*certain) * rvr_ray_sprite_stack.data_used);
      int16_t far = 0;
      certain[far] = 1;
      int done = 0;

      do
      {
         done = 1;
         for(int i = 0; i<rvr_ray_sprite_stack.data_used; i++)
         {
            if(certain[i])
               continue;

            int order = rvr_ray_sprite_order(i, far);
            if(order<0)
               continue;

            certain[i] = 1;
            if(!order)
            {
               done = 0;
               far = i;
               certain[far] = 1;
            }
         }
      }while(!done);

      if(rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_proxy[far]].flags & 8)
         rvr_ray_sprite_draw_wall(&rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_proxy[far]]);
      else if(rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_proxy[far]].flags & 16)
         rvr_ray_sprite_draw_floor(&rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_proxy[far]]);
      else
         rvr_ray_sprite_draw_billboard(&rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_proxy[far]]);

      rvr_ray_sprite_stack.data_used--;
      rvr_ray_sprite_stack.data_proxy[far] = rvr_ray_sprite_stack.data_proxy[rvr_ray_sprite_stack.data_used];
   }

   rvr_ray_sprite_stack.data_used = 0;
}

RvR_ray_depth_buffer *RvR_ray_draw_depth_buffer()
{
   return &rvr_ray_depth_buffer;
}

//raycastlib by drummyfish: https://gitlab.com/drummyfish/raycastlib
//Heavily modified: more efficient wall rendering, added floor/ceiling rendering
//Original header:
//raycastlib (RCL) - Small C header-only raycasting library for embedded and
//low performance computers, such as Arduino. Only uses integer math and stdint
//standard library.

void RvR_ray_draw_sprite(RvR_fix22_vec3 pos, RvR_fix22 angle, uint16_t tex, uint32_t flags)
{
   //Sprite tagged as invisible --> don't draw
   if(flags & 1)
      return;

   rvr_ray_sprite sprite_new = {0};
   sprite_new.texture = tex;
   sprite_new.flags = flags;

   //Wall alligned sprite
   if(flags & 8)
   {
      //Translate sprite to world space
      RvR_fix22_vec2 dir = RvR_fix22_vec2_rot(angle);
      RvR_fix22 half_width = RvR_texture_get(tex)->width * 8;
      sprite_new.p0.x = (-dir.y * half_width) / 1024 + pos.x;
      sprite_new.p0.y = (dir.x * half_width) / 1024 + pos.y;
      sprite_new.p1.x = (dir.y * half_width) / 1024 + pos.x;
      sprite_new.p1.y = (-dir.x * half_width) / 1024 + pos.y;
      sprite_new.p = pos;
      sprite_new.st0 = 0;
      sprite_new.st1 = 1023;
      sprite_new.angle = angle;

      //Translate to camera space
      RvR_fix22 x0 = sprite_new.p0.x - RvR_ray_get_position().x;
      RvR_fix22 y0 = sprite_new.p0.y - RvR_ray_get_position().y;
      RvR_fix22 x1 = sprite_new.p1.x - RvR_ray_get_position().x;
      RvR_fix22 y1 = sprite_new.p1.y - RvR_ray_get_position().y;
      RvR_fix22_vec2 to_point0;
      RvR_fix22_vec2 to_point1;
      to_point0.x = (-x0 * rvr_ray_sin + y0 * rvr_ray_cos) / 1024;
      to_point0.y = (x0 * rvr_ray_cos_fov + y0 * rvr_ray_sin_fov) / 1024;
      to_point1.x = (-x1 * rvr_ray_sin + y1 * rvr_ray_cos) / 1024;
      to_point1.y = (x1 * rvr_ray_cos_fov + y1 * rvr_ray_sin_fov) / 1024;

      //Behind camera
      if(to_point0.y<-128&&to_point1.y<-128)
         return;

      //Sprite not facing camera
      //--> swap p0 and p1 and toggle y-axis mirror flag
      if((to_point0.x * to_point1.y - to_point1.x * to_point0.y) / 65536>0)
      {
         RvR_fix22_vec2 tmp = to_point0;
         to_point0 = to_point1;
         to_point1 = tmp;
         sprite_new.flags ^= 2;
      }

      //Here we can treat everything as if we have a 90 degree
      //fov, since the rotation to camera space transforms it to
      //that
      //Check if in fov
      //Left point in fov
      if(to_point0.x>=-to_point0.y)
      {
         //Sprite completely out of sight
         if(to_point0.x>to_point0.y)
            return;

         sprite_new.sp0.x = RvR_min(RVR_XRES / 2 + (to_point0.x * (RVR_XRES / 2)) / to_point0.y, RVR_XRES - 1);
         sprite_new.sp0.z = to_point0.y;
      }
      //Left point to the left of fov
      else
      {
         //Sprite completely out of sight
         if(to_point1.x<-to_point1.y)
            return;

         sprite_new.sp0.x = 0;

         //Basically just this equation: (0,0)+n(-1,1) = (to_point0.x,to_point0.y)+m(to_point1.x-to_point0.x,to_point1.y-to_point0.y), reordered to n = ...
         //This is here to circumvent a multiplication overflow while also minimizing the resulting error
         //TODO: the first version might be universally better, making the if statement useless
         RvR_fix22 dx0 = to_point1.x - to_point0.x;
         RvR_fix22 dx1 = to_point0.y + to_point0.x;
         if(RvR_abs(dx0)>RvR_abs(dx1))
            sprite_new.sp0.z = (dx1 * ((dx0 * 1024) / RvR_non_zero(to_point1.y - to_point0.y + to_point1.x - to_point0.x))) / 1024 - to_point0.x;
         else
            sprite_new.sp0.z = (dx0 * ((dx1 * 1024) / RvR_non_zero(to_point1.y - to_point0.y + to_point1.x - to_point0.x))) / 1024 - to_point0.x;

         sprite_new.st0 = ((to_point0.x + to_point0.y) * 1024) / RvR_non_zero(-to_point1.y + to_point0.y - to_point1.x + to_point0.x);
      }

      //Right point in fov
      if(to_point1.x<=to_point1.y)
      {
         //sprite completely out of sight
         if(to_point1.x<-to_point1.y)
            return;

         sprite_new.sp1.x = RvR_min(RVR_XRES / 2 + (to_point1.x * (RVR_XRES / 2)) / to_point1.y - 1, RVR_XRES - 1);
         sprite_new.sp1.z = to_point1.y;
      }
      //Right point to the right of fov
      else
      {
         //sprite completely out of sight
         if(to_point0.x>to_point0.y)
            return;

         sprite_new.sp1.x = RVR_XRES - 1;

         //Basically just this equation: (0,0)+n(1,1) = (to_point0.x,to_point0.y)+m(to_point1.x-to_point0.x,to_point1.y-to_point0.y), reordered to n = ...
         //This is here to circumvent a multiplication overflow while also minimizing the resulting error
         //TODO: the first version might be universally better, making the if statement useless
         RvR_fix22 dx0 = to_point1.x - to_point0.x;
         RvR_fix22 dx1 = to_point0.y - to_point0.x;
         if(RvR_abs(dx0)>RvR_abs(dx1))
            sprite_new.sp1.z = to_point0.x - (dx1 * ((dx0 * 1024) / RvR_non_zero(to_point1.y - to_point0.y - to_point1.x + to_point0.x))) / 1024;
         else
            sprite_new.sp1.z = to_point0.x - (dx0 * ((dx1 * 1024) / RvR_non_zero(to_point1.y - to_point0.y - to_point1.x + to_point0.x))) / 1024;

         sprite_new.st1 = ((to_point0.y - to_point0.x) * 1024) / RvR_non_zero(-to_point1.y + to_point0.y + to_point1.x - to_point0.x);
      }

      //Near clip sprite
      if(sprite_new.sp0.z<16||sprite_new.sp1.z<16)
         return;

      //Far clip sprite
      if(sprite_new.sp0.z>RVR_RAY_MAX_STEPS * 1024&&sprite_new.sp1.z>RVR_RAY_MAX_STEPS * 1024)
         return;

      if(sprite_new.sp0.x>sprite_new.sp1.x)
         return;

      sprite_new.sp0.y = ((sprite_new.p.z - RvR_ray_get_position().z) * 1024) / RvR_non_zero((rvr_ray_fov_factor_y * sprite_new.sp0.z) / 1024);
      sprite_new.sp0.y = RVR_YRES * 512 - RVR_YRES * sprite_new.sp0.y + RvR_ray_get_shear() * 1024;
      sprite_new.sp1.y = ((sprite_new.p.z - RvR_ray_get_position().z) * 1024) / RvR_non_zero((rvr_ray_fov_factor_y * sprite_new.sp1.z) / 1024);
      sprite_new.sp1.y = RVR_YRES * 512 - RVR_YRES * sprite_new.sp1.y + RvR_ray_get_shear() * 1024;

      if(sprite_new.flags & 2)
      {
         sprite_new.st0 = 1023 - sprite_new.st0;
         sprite_new.st1 = 1023 - sprite_new.st1;
      }

      rvr_ray_sprite_stack_push(sprite_new);

      return;
   }

   //Floor alligned sprite
   if(flags & 16)
   {
      return;
   }

   //Billboarded sprite

   //Translate sprite to world space coordinates
   RvR_fix22 half_width = (RvR_texture_get(tex)->width * 1024 / 64) / 2;
   sprite_new.p0.x = (-rvr_ray_sin * half_width) / 1024 + pos.x;
   sprite_new.p0.y = (rvr_ray_cos * half_width) / 1024 + pos.y;
   sprite_new.p1.x = (rvr_ray_sin * half_width) / 1024 + pos.x;
   sprite_new.p1.y = (-rvr_ray_cos * half_width) / 1024 + pos.y;
   sprite_new.p = pos;

   //Translate to camera space
   RvR_fix22 x0 = sprite_new.p0.x - RvR_ray_get_position().x;
   RvR_fix22 y0 = sprite_new.p0.y - RvR_ray_get_position().y;
   RvR_fix22 x1 = sprite_new.p1.x - RvR_ray_get_position().x;
   RvR_fix22 y1 = sprite_new.p1.y - RvR_ray_get_position().y;
   RvR_fix22_vec2 to_point0;
   RvR_fix22_vec2 to_point1;
   to_point0.x = (-x0 * rvr_ray_sin + y0 * rvr_ray_cos) / 1024;
   to_point0.y = (x0 * rvr_ray_cos_fov + y0 * rvr_ray_sin_fov) / 1024;
   to_point1.x = (-x1 * rvr_ray_sin + y1 * rvr_ray_cos) / 1024;
   to_point1.y = (x1 * rvr_ray_cos_fov + y1 * rvr_ray_sin_fov) / 1024;

   //Behind camera
   if(to_point0.y<-128&&to_point1.y<-128)
      return;

   //Sprite not facing camera
   //--> swap p0 and p1
   if((to_point0.x * to_point1.y - to_point1.x * to_point0.y) / 65536>0)
   {
      RvR_fix22_vec2 tmp = to_point0;
      to_point0 = to_point1;
      to_point1 = tmp;
   }

   //Here we can treat everything as if we have a 90 degree
   //fov, since the rotation to camera space transforms it to
   //that
   //Check if in fov
   //Left point in fov
   if(to_point0.x>=-to_point0.y)
   {
      if(to_point0.x>to_point0.y)
         return;

      sprite_new.sp0.x = RvR_min(RVR_XRES / 2 + (to_point0.x * (RVR_XRES / 2)) / to_point0.y, RVR_XRES - 1);
   }
   //Left point to the left of fov
   else
   {
      if(to_point1.x<-to_point1.y)
         return;

      sprite_new.sp0.x = 0;
   }

   //Right point in fov
   if(to_point1.x<=to_point1.y)
   {
      if(to_point1.x<-to_point1.y)
         return;

      sprite_new.sp1.x = RvR_min(RVR_XRES / 2 + (to_point1.x * (RVR_XRES / 2)) / to_point1.y - 1, RVR_XRES - 1);
   }
   else
   {
      if(to_point0.x>to_point0.y)
         return;
      sprite_new.sp1.x = RVR_XRES - 1;
   }

   //Project to screen
   RvR_ray_pixel_info p = RvR_ray_map_to_screen(sprite_new.p);
   sprite_new.sp.x = p.position.x;
   sprite_new.sp.y = p.position.y;
   sprite_new.sp.z = p.depth;

   //Clipping
   //Behind camera
   if(sprite_new.sp.z<=0)
      return;
   //Too far away
   if(sprite_new.sp.z>RVR_RAY_MAX_STEPS * 1024)
      return;

   rvr_ray_sprite_stack_push(sprite_new);
}

void RvR_ray_draw_map()
{
   RvR_fix22 ray_span_start[RVR_YRES];

   //Render walls and fill plane data
   RvR_rays_cast_multi_hit_draw(rvr_ray_draw_column);
   //-------------------------------------

   //Render floor planes
   for(int i = 0; i<128; i++)
   {
      rvr_ray_plane *pl = rvr_ray_planes[i];
      while(pl!=NULL)
      {
         if(pl->min>pl->max)
            goto next;

         //Sky texture is rendered differently (vertical collumns instead of horizontal ones)
         if(pl->tex==RvR_ray_map_sky_tex())
         {
            RvR_texture *texture = RvR_texture_get(RvR_ray_map_sky_tex());
            int skyw = 1 << RvR_log2(texture->width);
            int skyh = 1 << RvR_log2(texture->height);
            int mask = skyh - 1;

            RvR_fix22 angle_step = (skyw * 1024) / RVR_XRES;
            RvR_fix22 tex_step = (1024 * skyh - 1) / RVR_YRES;

            RvR_fix22 angle = (RvR_ray_get_angle()) * 256;
            angle += (pl->min - 1) * angle_step;

            for(int x = pl->min; x<pl->max + 1; x++)
            {
               //Sky is rendered fullbright, no lut needed
               uint8_t * restrict pix = &RvR_core_framebuffer()[(pl->start[x]) * RVR_XRES + x - 1];
               const uint8_t * restrict tex = &texture->data[((angle >> 10) & (skyw - 1)) * skyh];
               const uint8_t * restrict col = RvR_shade_table(32);

               //Split in two parts: above and below horizon
               int middle = RvR_max(0, RvR_min(RVR_YRES, rvr_ray_middle_row + RVR_YRES / 32));
               int tex_start = pl->start[x];
               int tex_end = middle;
               if(tex_end>pl->end[x])
                  tex_end = pl->end[x];
               if(tex_start>tex_end)
                  tex_end = tex_start;
               if(tex_start>middle)
                  tex_end = tex_start - 1;
               int solid_end = pl->end[x];
               RvR_fix22 texture_coord = (RVR_YRES - middle + pl->start[x]) * tex_step;

               for(int y = tex_start; y<tex_end + 1; y++)
               {
                  *pix = tex[texture_coord >> 10];
                  texture_coord += tex_step;
                  pix += RVR_XRES;
               }
               RvR_fix22 tex_coord = (RVR_YRES)*tex_step - 1;
               texture_coord = RvR_min(tex_coord, tex_coord - tex_step * (tex_end - middle));
               for(int y = tex_end + 1; y<solid_end + 1; y++)
               {
                  *pix = col[tex[(texture_coord >> 10) & mask]];
                  texture_coord -= tex_step;
                  pix += RVR_XRES;
               }

               angle += angle_step;
            }

            goto next;
         }

         //Convert plane to horizontal spans
         RvR_texture *texture = RvR_texture_get(pl->tex);
         for(int x = pl->min; x<pl->max + 2; x++)
         {
            RvR_fix22 s0 = pl->start[x - 1];
            RvR_fix22 s1 = pl->start[x];
            RvR_fix22 e0 = pl->end[x - 1];
            RvR_fix22 e1 = pl->end[x];

            //End spans top
            for(; s0<s1&&s0<=e0; s0++)
            {
#if RVR_RAY_DRAW_PLANES==1
               rvr_ray_span_draw_flat(ray_span_start[s0], x - 1, s0, (i + 1) & 255);
#elif RVR_RAY_DRAW_PLANES==2
               rvr_ray_span_draw_tex(ray_span_start[s0], x - 1, s0, pl->height, texture);
#endif
            }

            //End spans bottom
            for(; e0>e1&&e0>=s0; e0--)
            {
#if RVR_RAY_DRAW_PLANES==1
               rvr_ray_span_draw_flat(ray_span_start[e0], x - 1, e0, (i + 1) & 255);
#elif RVR_RAY_DRAW_PLANES==2
               rvr_ray_span_draw_tex(ray_span_start[e0], x - 1, e0, pl->height, texture);
#endif
            }

            //Start spans top
            for(; s1<s0&&s1<=e1; s1++)
               ray_span_start[s1] = x - 1;

            //Start spans bottom
            for(; e1>e0&&e1>=s1; e1--)
               ray_span_start[e1] = x - 1;
         }

next:
         pl = pl->next;
      }
   }
   //-------------------------------------
}

void RvR_ray_draw_debug(uint8_t index)
{
   char tmp[128];
   snprintf(tmp, 128, "%03d.%01d ms\n", RvR_core_frametime_average() / 10, RvR_core_frametime_average() % 10);
   RvR_draw_string(2, 2, 1, tmp, index);

   for(int i = 0; i<128; i++)
   {
      int num = 0;
      rvr_ray_plane *pl = rvr_ray_planes[i];
      while(pl!=NULL)
      {
         pl = pl->next;
         num++;
      }
      RvR_draw_vertical_line(RVR_XRES - 128 + i, RVR_YRES - num, RVR_YRES, index);
   }
}

static int16_t rvr_ray_draw_wall(RvR_fix22 y_current, RvR_fix22 y_from, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, RvR_fix22 height, int16_t increment, RvR_ray_pixel_info *pixel_info, RvR_ray_hit_result *hit)
{
   int16_t limit = RvR_clamp(y_to, limit0, limit1);
   int start = 0;
   int end = 0;

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
   if(increment==-1&&hit->wall_ftex==RvR_ray_map_sky_tex())
   {
      rvr_ray_plane_add(hit->fheight, hit->wall_ftex, pixel_info->position.x, start, end);
      return limit;
   }
   else if(increment==1&&hit->wall_ctex==RvR_ray_map_sky_tex())
   {
      rvr_ray_plane_add(hit->cheight, hit->wall_ctex, pixel_info->position.x, start, end);
      return limit;
   }

   RvR_texture *texture = NULL;
   RvR_fix22 coord_step_scaled = (4 * rvr_ray_fov_factor_y * pixel_info->depth) / RVR_YRES;
   RvR_fix22 texture_coord_scaled = height * 4096 + (start - rvr_ray_middle_row + 1) * coord_step_scaled;

   if(increment==-1)
      texture = RvR_texture_get(hit->wall_ftex);
   else if(increment==1)
      texture = RvR_texture_get(hit->wall_ctex);

   uint8_t * restrict pix = &RvR_core_framebuffer()[start * RVR_XRES + pixel_info->position.x];
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63, (hit->direction & 1) * 10 + (pixel_info->depth >> 9)));
   const uint8_t * restrict tex = &texture->data[(hit->texture_coord >> 4) * texture->height];
   RvR_fix22 y_and = (1 << RvR_log2(texture->height)) - 1;

#if RVR_UNROLL

   int count = end - start + 1;
   int n = (count + 7) / 8;
   switch(count % 8)
   {
   case 0: do {
         *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
         case 7: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
         case 6: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
         case 5: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
         case 4: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
         case 3: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
         case 2: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
         case 1: *pix = col[tex[(texture_coord_scaled >> 16) & y_and]]; texture_coord_scaled += coord_step_scaled; pix += RVR_XRES; //fallthrough
   }while(--n>0);
   }

#else

   for(int i = start; i<=end; i++)
   {
      *pix = col[tex[(texture_coord_scaled >> 16) & y_and]];
      texture_coord_scaled += coord_step_scaled;
      pix += RVR_XRES;
   }

#endif

   return limit;
}

static void rvr_ray_draw_column(RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray)
{
   //last written Y position, can never go backwards
   RvR_fix22 f_pos_y = RVR_YRES;
   RvR_fix22 c_pos_y = -1;

   //world coordinates (relative to camera height though)
   RvR_fix22 f_z1_world = rvr_ray_start_floor_height;
   RvR_fix22 c_z1_world = rvr_ray_start_ceil_height;

   RvR_ray_pixel_info p = {0};
   RvR_ray_hit_result h = {0};
   p.position.x = x;

   //we'll be simulatenously drawing the floor and the ceiling now
   for(RvR_fix22 j = 0; j<=hits_len; ++j)
   {                    //^ = add extra iteration for horizon plane
      int8_t drawing_horizon = j==(hits_len);
      int limit_c = 0;
      int limit_f = 0;

      RvR_fix22 distance = 0;

      RvR_fix22 f_z2_world = 0,    c_z2_world = 0;
      RvR_fix22 f_z1_screen = 0,   c_z1_screen = 0;
      RvR_fix22 f_z2_screen = 0,   c_z2_screen = 0;

      if(!drawing_horizon)
      {
         RvR_ray_hit_result hit = hits[j];
         distance = RvR_non_zero(hit.distance);
         h = hit;

         RvR_fix22 wall_height = RvR_ray_map_floor_height_at(hit.square.x, hit.square.y);
         f_z2_world = wall_height - RvR_ray_get_position().z;
         f_z1_screen = rvr_ray_middle_row - ((f_z1_world * RVR_YRES) / RvR_non_zero((rvr_ray_fov_factor_y * distance) / 1024));
         f_z2_screen = rvr_ray_middle_row - ((f_z2_world * RVR_YRES) / RvR_non_zero((rvr_ray_fov_factor_y * distance) / 1024));

         wall_height = RvR_ray_map_ceiling_height_at(hit.square.x, hit.square.y);
         c_z2_world = wall_height - RvR_ray_get_position().z;
         c_z1_screen = rvr_ray_middle_row - ((c_z1_world * RVR_YRES) / RvR_non_zero((rvr_ray_fov_factor_y * distance) / 1024));
         c_z2_screen = rvr_ray_middle_row - ((c_z2_world * RVR_YRES) / RvR_non_zero((rvr_ray_fov_factor_y * distance) / 1024));
      }
      else
      {
         f_z1_screen = rvr_ray_middle_row;
         c_z1_screen = rvr_ray_middle_row + 1;

         h = hits[j - 1];
         h.distance = 1024 * 1024;
         /* ^ horizon is at infinity, but we can't use too big infinity
              because it would overflow in the following mult. */
         h.position.x = (ray.direction.x * h.distance) / 1024;
         h.position.y = (ray.direction.y * h.distance) / 1024;

         h.direction = 0;
         h.texture_coord = 0;
         h.wall_ftex = RvR_ray_map_sky_tex();
         h.wall_ctex = RvR_ray_map_sky_tex();
         h.floor_tex = RvR_ray_map_sky_tex();
         h.ceil_tex = RvR_ray_map_sky_tex();
         h.fheight = RvR_fix22_infinity;
         h.cheight = RvR_fix22_infinity;
      }

      RvR_fix22 limit;

      //draw floor until wall
      limit_f = limit = RvR_clamp(f_z1_screen, c_pos_y + 1, RVR_YRES);
      if(f_pos_y>limit)
      {
         rvr_ray_plane_add(h.fheight, h.floor_tex, p.position.x, limit, f_pos_y - 1);
         f_pos_y = limit;
      }

      //draw ceiling until wall
      limit_c = limit = RvR_clamp(c_z1_screen, -1, f_pos_y - 1);
      if(limit>c_pos_y)
      {
         rvr_ray_plane_add(h.cheight, h.ceil_tex, p.position.x, c_pos_y + 1, limit);
         c_pos_y = limit;
      }

      if(!drawing_horizon) //don't draw walls for horizon plane
      {
         p.depth = distance;
         p.depth = RvR_max(0, RvR_min(p.depth, (RVR_RAY_MAX_STEPS) * 1024));

         //draw floor wall
         if(f_z1_world!=f_z2_world)
         {
            if(f_pos_y>0)  //still pixels left?
            {
               limit = rvr_ray_draw_wall(f_pos_y, f_z1_screen, f_z2_screen, c_pos_y + 1,
                                         RVR_YRES,
                                         //^ purposfully allow outside screen bounds here
                                         f_z2_world
                                         , -1, &p, &h);
               if(f_pos_y>limit)
                  f_pos_y = limit;

               f_z1_world = f_z2_world; //for the next iteration
               //^ purposfully allow outside screen bounds here
            }

            int limit_clip = RvR_min(limit, limit_f);
            RvR_ray_depth_buffer_entry *entry = rvr_ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip;
            entry->next = rvr_ray_depth_buffer.floor[p.position.x];
            rvr_ray_depth_buffer.floor[p.position.x] = entry;
         }

         //draw ceiling wall
         if(c_z1_world!=c_z2_world)
         {
            if(c_pos_y<RVR_YRES - 1) //pixels left?
            {
               limit = rvr_ray_draw_wall(c_pos_y, c_z1_screen, c_z2_screen,
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
            RvR_ray_depth_buffer_entry *entry = rvr_ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip + 1;
            entry->next = rvr_ray_depth_buffer.ceiling[p.position.x];
            rvr_ray_depth_buffer.ceiling[p.position.x] = entry;
         }
      }
   }
}

RvR_ray_pixel_info RvR_ray_map_to_screen(RvR_fix22_vec3 world_position)
{
   RvR_ray_pixel_info result;

   RvR_fix22_vec2 to_point;

   to_point.x = world_position.x - RvR_ray_get_position().x;
   to_point.y = world_position.y - RvR_ray_get_position().y;

   RvR_fix22 middle_column = RVR_XRES / 2;

   //rotate the point to camera space (y left/right, x forw/backw)
   RvR_fix22 tmp = to_point.x;
   to_point.x = (to_point.x * rvr_ray_cos + to_point.y * rvr_ray_sin) / 1024;
   to_point.y = (tmp * rvr_ray_sin - to_point.y * rvr_ray_cos) / 1024;

   result.depth = to_point.x;

   result.position.x = middle_column - (((to_point.y * 1024) / RvR_non_zero((rvr_ray_fov_factor_x * RvR_abs(result.depth)) / 1024)) * middle_column) / 1024;
   result.position.y = ((((world_position.z - RvR_ray_get_position().z) * 1024) / RvR_non_zero((rvr_ray_fov_factor_y * result.depth) / 1024)) * RVR_YRES) / 1024;
   result.position.y = RVR_YRES / 2 - result.position.y + RvR_ray_get_shear();

   return result;
}

static void rvr_ray_plane_add(RvR_fix22 height, uint16_t tex, int x, int y0, int y1)
{
   x += 1;
   //Div height by 128, since it's usually in these increments
   int hash = ((height >> 7) * 7 + tex * 3) & 127;

   rvr_ray_plane *pl = rvr_ray_planes[hash];
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
         if(pl->start[x] - 1==y1)
         {
            pl->start[x] = y0;
            return;
         }
         if(pl->end[x] + 1==y0)
         {
            pl->end[x] = y1;
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
      pl = rvr_ray_plane_new();
      pl->next = rvr_ray_planes[hash];
      rvr_ray_planes[hash] = pl;

      pl->min = RVR_XRES;
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

   pl->end[x] = y1;
   pl->start[x] = y0;
}

static void rvr_ray_span_draw_tex(int x0, int x1, int y, RvR_fix22 height, const RvR_texture *texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   //Calculate the depth of the row to be rendered
   RvR_fix22 dy = rvr_ray_middle_row - y;
   RvR_fix22 depth = (RvR_abs(RvR_ray_get_position().z - height) * 1024) / RvR_non_zero(rvr_ray_fov_factor_y);
   depth = (depth * RVR_YRES) / RvR_non_zero(RvR_abs(dy));

   //Calculate texture mapping step size and starting coordinates
   RvR_fix22 step_x = ((rvr_ray_sin * (RvR_ray_get_position().z - height))) / RvR_non_zero((dy));
   RvR_fix22 step_y = ((rvr_ray_cos * (RvR_ray_get_position().z - height))) / RvR_non_zero((dy));
   //RvR_fix22 step_x = ((rvr_ray_sin*(RvR_ray_get_position().z-height))+(dy)/2)/RvR_non_zero((dy));
   //RvR_fix22 step_y = ((rvr_ray_cos*(RvR_ray_get_position().z-height))+(dy)/2)/RvR_non_zero((dy));
   RvR_fix22 tx = (RvR_ray_get_position().x & 1023) * 1024 + rvr_ray_cos * depth + ((x0 - RVR_XRES / 2) * step_x);
   RvR_fix22 ty = -(RvR_ray_get_position().y & 1023) * 1024 - rvr_ray_sin * depth + ((x0 - RVR_XRES / 2) * step_y);

   uint8_t * restrict pix = &RvR_core_framebuffer()[y * RVR_XRES + x0];
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63, (depth >> 9)));
   const uint8_t * restrict tex = texture->data;
   RvR_fix22 x_and = (1 << RvR_log2(texture->width)) - 1;
   RvR_fix22 y_and = (1 << RvR_log2(texture->height)) - 1;
   RvR_fix22 y_log2 = RvR_log2(texture->height);

#if RVR_UNROLL

   int count = x1 - x0;
   int n = (count + 7) / 8;
   switch(count % 8)
   {
   case 0: do {
         *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
         case 7: *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
         case 6: *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
         case 5: *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
         case 4: *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
         case 3: *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
         case 2: *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
         case 1: *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]]; tx += step_x; ty += step_y; pix++; //fallthrough
   }while(--n>0);
   }

#else

   for(int x = x0; x<x1; x++)
   {
      *pix = col[tex[(((tx >> 14) & x_and) << y_log2) + ((ty >> 14) & y_and)]];
      tx += step_x;
      ty += step_y;
      pix++;
   }

#endif
}

static void rvr_ray_span_draw_flat(int x0, int x1, int y, uint8_t color)
{
   uint8_t * restrict pix = &RvR_core_framebuffer()[y * RVR_XRES + x0];

#if RVR_UNROLL

   int count = x1 - x0;
   int n = (count + 7) / 8;
   switch(count % 8)
   {
   case 0: do {
         *pix = color; pix++;   //fallthrough
         case 7: *pix = color; pix++; //fallthrough
         case 6: *pix = color; pix++; //fallthrough
         case 5: *pix = color; pix++; //fallthrough
         case 4: *pix = color; pix++; //fallthrough
         case 3: *pix = color; pix++; //fallthrough
         case 2: *pix = color; pix++; //fallthrough
         case 1: *pix = color; pix++; //fallthrough
   }while(--n>0);
   }

#else

   for(int x = x0; x<x1; x++)
   {
      *pix = color;
      pix++;
   }

#endif
}

static void rvr_ray_sprite_stack_push(rvr_ray_sprite s)
{
   if(rvr_ray_sprite_stack.data==NULL)
   {
      rvr_ray_sprite_stack.data_size = 64;
      rvr_ray_sprite_stack.data = RvR_malloc(sizeof(*rvr_ray_sprite_stack.data) * rvr_ray_sprite_stack.data_size);
      rvr_ray_sprite_stack.data_proxy = RvR_malloc(sizeof(*rvr_ray_sprite_stack.data_proxy) * rvr_ray_sprite_stack.data_size);
   }

   rvr_ray_sprite_stack.data_proxy[rvr_ray_sprite_stack.data_used] = rvr_ray_sprite_stack.data_used;
   rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_used++] = s;

   if(rvr_ray_sprite_stack.data_used==rvr_ray_sprite_stack.data_size)
   {
      rvr_ray_sprite_stack.data_size += 64;
      rvr_ray_sprite_stack.data = RvR_realloc(rvr_ray_sprite_stack.data, sizeof(*rvr_ray_sprite_stack.data) * rvr_ray_sprite_stack.data_size);
      rvr_ray_sprite_stack.data_proxy = RvR_realloc(rvr_ray_sprite_stack.data_proxy, sizeof(*rvr_ray_sprite_stack.data_proxy) * rvr_ray_sprite_stack.data_size);
   }
}

//Returns:
//-1 sprites don't obstruct each other
//0 sprite a is obstructed by sprite b --> sprite a first
//1 sprite b is obstructed by sprite a --> sprite b first
static int rvr_ray_sprite_order(int a, int b)
{
   rvr_ray_sprite *sa = &rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_proxy[a]];
   rvr_ray_sprite *sb = &rvr_ray_sprite_stack.data[rvr_ray_sprite_stack.data_proxy[b]];

   if(sa->sp0.x>=sb->sp1.x||sb->sp0.x>=sa->sp1.x)
      return -1;

   //Shortcut if both are billboarded sprites
   if(!(sa->flags & 8)&&!(sb->flags & 8))
      return sa->sp.z<sb->sp.z;

   RvR_fix22 x00 = sa->p0.x;
   RvR_fix22 y00 = sa->p0.y;
   RvR_fix22 x01 = sa->p1.x;
   RvR_fix22 y01 = sa->p1.y;
   RvR_fix22 x10 = sb->p0.x;
   RvR_fix22 y10 = sb->p0.y;
   RvR_fix22 x11 = sb->p1.x;
   RvR_fix22 y11 = sb->p1.y;

   //(x00/y00) is origin, all calculations centered arround it
   //t0 is relation of (b,p0) to sprite a
   //t1 is relation of (b,p1) to sprite a
   RvR_fix22 x = x01 - x00;
   RvR_fix22 y = y01 - y00;
   RvR_fix22 t0 = ((x10 - x00) * y - (y10 - y00) * x) / 1024;
   RvR_fix22 t1 = ((x11 - x00) * y - (y11 - y00) * x) / 1024;

   //sprite on the same line (identicall or adjacent)
   if(t0==0&&t1==0)
      return -1;

   //(b,p0) on extension of sprite a (shared corner, etc)
   //Set t0 = t1 to trigger RvR_sign_equal check (and for the return !RvR_sign_equal to be correct)
   if(t0==0)
      t0 = t1;

   //(b,p1) on extension of sprite a
   //Set t0 = t1 to trigger RvR_sign_equal check
   if(t1==0)
      t1 = t0;

   //Sprite either completely to the left or to the right of other wall
   if(RvR_sign_equal(t0, t1))
   {
      //Compare with player position relative to sprite a
      //if sprite b and the player share the same relation, sprite a needs to be drawn first
      t1 = ((RvR_ray_get_position().x - x00) * y - (RvR_ray_get_position().y - y00) * x) / 1024;
      return !RvR_sign_equal(t0, t1);
   }

   //Extension of sprite a intersects with sprite b
   //--> check sprite b instead
   //(x10/y10) is origin, all calculations centered arround it
   x = x11 - x10;
   y = y11 - y10;
   t0 = ((x00 - x10) * y - (y00 - y10) * x) / 1024;
   t1 = ((x01 - x10) * y - (y01 - y10) * x) / 1024;

   //sprite on the same line (identicall or adjacent)
   if(t0==0&&t1==0)
      return -1;

   //(a,p0) on extension of sprite b
   if(t0==0)
      t0 = t1;

   //(a,p1) on extension of sprite b
   if(t1==0)
      t1 = t0;

   //sprite either completely to the left or to the right of other sprite
   if(RvR_sign_equal(t0, t1))
   {
      //Compare with player position relative to sprite b
      //if sprite a and the player share the same relation, sprite b needs to be drawn first
      t1 = ((RvR_ray_get_position().x - x10) * y - (RvR_ray_get_position().y - y10) * x) / 1024;
      return RvR_sign_equal(t0, t1);
   }

   //Invalid case: sprites are intersecting
   return -1;
}

static void rvr_ray_sprite_draw_wall(rvr_ray_sprite *sp)
{
   int x0 = sp->sp0.x;
   int x1 = sp->sp1.x;

   //Shouldn't happen
   if(x1<0||x0>x1||x0>RVR_XRES)
      return;

   RvR_texture *texture = RvR_texture_get(sp->texture);
   int mask = (1 << RvR_log2(texture->height)) - 1;
   RvR_fix22 scale_vertical = texture->height * 16;
   int size0 = RVR_YRES * ((scale_vertical * 1024) / RvR_non_zero((rvr_ray_fov_factor_y * sp->sp0.z) / 1024));
   int size1 = RVR_YRES * ((scale_vertical * 1024) / RvR_non_zero((rvr_ray_fov_factor_y * sp->sp1.z) / 1024));
   int y0 = sp->sp0.y;
   int y1 = sp->sp1.y;

   //Floor and ceiling clip
   int clip_bottom = RVR_YRES;
   int clip_top = 0;
   /*RvR_fix22_vec3 floor_wpos;
   floor_wpos.x = sp->p.x;
   floor_wpos.y = sp->p.y;
   floor_wpos.z = RvR_ray_map_floor_height_at(sp->p.x/1024,sp->p.y/1024);
   int clip_bottom = RvR_ray_map_to_screen(floor_wpos).position.y;
   floor_wpos.z = RvR_ray_map_ceiling_height_at(sp->p.x/1024,sp->p.y/1024);
   int clip_top = RvR_ray_map_to_screen(floor_wpos).position.y;
   clip_bottom = clip_bottom>RVR_YRES?RVR_YRES:clip_bottom;
   clip_top = clip_top<0?0:clip_top;*/

   RvR_fix22 depth0 = INT32_MAX / RvR_non_zero(sp->sp0.z);
   RvR_fix22 depth1 = INT32_MAX / RvR_non_zero(sp->sp1.z);
   RvR_fix22 step_depth = (depth1 - depth0) / RvR_non_zero(x1 - x0);
   RvR_fix22 depth_i = depth0;

   RvR_fix22 st0 = (sp->st0 * 1024 * 1024) / RvR_non_zero(sp->sp0.z);
   RvR_fix22 st1 = (sp->st1 * 1024 * 1024) / RvR_non_zero(sp->sp1.z);
   RvR_fix22 step_u = (st1 - st0) / RvR_non_zero(x1 - x0);
   RvR_fix22 u_i = st0;

   RvR_fix22 t0 = y0 - size0 + 512;
   RvR_fix22 t1 = y1 - size1 + 512;
   RvR_fix22 step_t = (t1 - t0) / RvR_non_zero(x1 - x0);
   RvR_fix22 top = t0;

   RvR_fix22 b0 = y0 - 1024;
   RvR_fix22 b1 = y1 - 1024;
   RvR_fix22 step_b = (b1 - b0) / RvR_non_zero(x1 - x0);
   RvR_fix22 bot = b0;

   const uint8_t * restrict col = NULL;
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;

   RvR_fix22 angle = RvR_ray_get_angle() - sp->angle;
   RvR_fix22 len = texture->width * 16;
   RvR_fix22 dx0 = sp->p1.x - sp->p0.x;
   RvR_fix22 dy0 = sp->p1.y - sp->p0.y;
   RvR_fix22 dx1 = RvR_ray_get_position().x - sp->p0.x;
   RvR_fix22 dy1 = RvR_ray_get_position().y - sp->p0.y;
   RvR_fix22 offset = (dx0 * dx1 + dy0 * dy1) / len;
   RvR_fix22 dist = (dy0 * dx1 - dx0 * dy1) / len;

   RvR_fix22 u_clamp = sp->st0 * 1024;
   int clamp_dir = sp->st0<sp->st1;
   for(int i = x0; i<=x1; i++)
   {
      RvR_fix22 depth = INT32_MAX / RvR_non_zero(depth_i);
      RvR_fix22 u = (u_i / 1024) * depth;

      //Most ugly fix ever
      //Stops the texture coordinate from skipping back and forth due to
      //fixed point inaccuracies by clamping it to the last value
      if(clamp_dir)
         u = RvR_max(u_clamp, u);
      else
         u = RvR_min(u_clamp, u);
      u_clamp = u;
      u *= texture->width;

      //RvR_fix22 tan = RvR_fix22_tan(angle+ray_x_to_angle[i]);
      //printf("%d\n",ray_x_to_angle[i]);
      //printf("%d %d\n",i,offset-(tan*dist)/1024);

      //printf("%d\n",(offset-(tan*depth)/1024)/16);

      RvR_fix22 step_v = (4 * rvr_ray_fov_factor_y * depth) / RVR_YRES;
      RvR_fix22 y = top / 1024;

      int sy = 0;
      int ey = bot / 1024 - top / 1024;
      if(y<clip_top)
         sy = clip_top - y;
      if(y + ey>clip_bottom)
         ey = (clip_bottom - y);
      y = y<clip_top?clip_top:y;

      int ys = y;
      int ye = ey;

      //Clip floor
      RvR_ray_depth_buffer_entry *clip = rvr_ray_depth_buffer.floor[i];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&y + (ye - sy)>clip->limit)
            ye = clip->limit - y + sy;
         clip = clip->next;
      }

      //Clip ceiling
      clip = rvr_ray_depth_buffer.ceiling[i];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
         {
            int diff = ys - clip->limit;
            ys = clip->limit;
            ye += diff;
         }
         clip = clip->next;
      }

      RvR_fix22 v = (sp->p.z - RvR_ray_get_position().z) * 4096 + (ys - rvr_ray_middle_row + 1) * step_v;

      tex = &texture->data[texture->height * (u >> 20)];
      dst = &RvR_core_framebuffer()[ys * RVR_XRES + i];
      col = RvR_shade_table(RvR_min(63, depth >> 9));

      if(sp->flags & 32)
      {
         for(int yi = sy; yi<ye; yi++, dst += RVR_XRES)
         {
            uint8_t index = tex[(v >> 16) & mask];
            *dst = RvR_blend(col[index], *dst);
            v += step_v;
         }
      }
      else if(sp->flags & 64)
      {
         for(int yi = sy; yi<ye; yi++, dst += RVR_XRES)
         {
            uint8_t index = tex[(v >> 16) & mask];
            *dst = RvR_blend(*dst, col[index]);
            v += step_v;
         }
      }
      else
      {
         for(int yi = sy; yi<ye; yi++, dst += RVR_XRES)
         {
            uint8_t index = tex[(v >> 16) & mask];
            *dst = index?col[index]:*dst;
            v += step_v;
         }
      }

      depth_i += step_depth;
      u_i += step_u;

      top += step_t;
      bot += step_b;
   }
}

static void rvr_ray_sprite_draw_floor(rvr_ray_sprite *sp)
{}

//TODO: reimplement flipped sprites
static void rvr_ray_sprite_draw_billboard(rvr_ray_sprite *sp)
{
   RvR_texture *texture = RvR_texture_get(sp->texture);

   RvR_fix22 tpx = sp->p.x - RvR_ray_get_position().x;
   RvR_fix22 tpy = sp->p.y - RvR_ray_get_position().y;
   RvR_fix22 depth = (tpx * rvr_ray_cos + tpy * rvr_ray_sin) / 1024;
   tpx = (tpx * rvr_ray_sin - tpy * rvr_ray_cos) / 1024;

   //Dimensions
   RvR_fix22 top = ((sp->p.z - RvR_ray_get_position().z + texture->height * 16) * 1024) / RvR_non_zero((depth * rvr_ray_fov_factor_y) / 1024);
   top = rvr_ray_middle_row * 1024 - top * RVR_YRES;
   int y0 = (top + 1023) / 1024;

   RvR_fix22 bot = ((sp->p.z - RvR_ray_get_position().z) * 1024) / RvR_non_zero((depth * rvr_ray_fov_factor_y) / 1024);
   bot = rvr_ray_middle_row * 1024 - bot * RVR_YRES;
   int y1 = (bot - 1) / 1024;

   RvR_fix22 left = ((tpx + texture->width * 8) * 1024) / RvR_non_zero((depth * rvr_ray_fov_factor_x) / 1024);
   left = RVR_XRES * 512 - left * (RVR_XRES / 2);
   int x0 = (left + 1023) / 1024;

   RvR_fix22 right = ((tpx - texture->width * 8) * 1024) / RvR_non_zero((depth * rvr_ray_fov_factor_x) / 1024);
   right = RVR_XRES * 512 - right * (RVR_XRES / 2);
   int x1 = (right - 1) / 1024;

   //Floor and ceiling clip
   RvR_fix22 cy = ((RvR_ray_map_floor_height_at(sp->p.x / 1024, sp->p.y / 1024) - RvR_ray_get_position().z) * 1024) / RvR_non_zero((depth * rvr_ray_fov_factor_y) / 1024);
   cy = rvr_ray_middle_row * 1024 - cy * RVR_YRES;
   int clip_bottom = RvR_min(cy / 1024, RVR_YRES);

   cy = ((RvR_ray_map_ceiling_height_at(sp->p.x / 1024, sp->p.y / 1024) - RvR_ray_get_position().z) * 1024) / RvR_non_zero((depth * rvr_ray_fov_factor_y) / 1024);
   cy = rvr_ray_middle_row * 1024 - cy * RVR_YRES;
   int clip_top = RvR_max(cy / 1024, 0);

   y0 = RvR_max(y0, clip_top);
   y1 = RvR_min(y1, clip_bottom);
   x1 = RvR_min(x1, RVR_XRES);
   RvR_fix22 step_v = (4 * rvr_ray_fov_factor_y * depth) / RVR_YRES;
   RvR_fix22 step_u = (8 * rvr_ray_fov_factor_x * depth) / RVR_XRES;
   RvR_fix22 u = (step_u * (x0 * 1024 - left)) / 1024;

   if(x0<0)
   {
      u += (-x0) * step_u;
      x0 = 0;
   }

   //Draw
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63, depth >> 9));
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;
   for(int x = x0; x<x1; x++)
   {
      //Clip against walls
      int ys = y0;
      int ye = y1;

      //Clip floor
      RvR_ray_depth_buffer_entry *clip = RvR_ray_draw_depth_buffer()->floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ye>clip->limit)
            ye = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      clip = RvR_ray_draw_depth_buffer()->ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
            ys = clip->limit;
         clip = clip->next;
      }

      tex = &texture->data[texture->height * (u >> 16)];
      dst = &RvR_core_framebuffer()[ys * RVR_XRES + x];
      RvR_fix22 v = (sp->p.z - RvR_ray_get_position().z) * 4096 + (ys - rvr_ray_middle_row + 1) * step_v + texture->height * 65536;

      if(sp->flags & 32)
      {
         for(int y = ys; y<ye; y++, dst += RVR_XRES)
         {
            uint8_t index = tex[v >> 16];
            *dst = RvR_blend(col[index], *dst);
            v += step_v;
         }
      }
      else if(sp->flags & 64)
      {
         for(int y = ys; y<ye; y++, dst += RVR_XRES)
         {
            uint8_t index = tex[v >> 16];
            *dst = RvR_blend(*dst, col[index]);
            v += step_v;
         }
      }
      else
      {
         for(int y = ys; y<ye; y++, dst += RVR_XRES)
         {
            uint8_t index = tex[v >> 16];
            *dst = index?col[index]:*dst;
            v += step_v;
         }
      }

      u += step_u;
   }
}

static rvr_ray_plane *rvr_ray_plane_new()
{
   if(rvr_ray_plane_pool==NULL)
   {
      rvr_ray_plane *p = RvR_malloc(sizeof(*p) * 8);
      memset(p, 0, sizeof(*p) * 8);

      for(int i = 0; i<7; i++)
         p[i].next = &p[i + 1];
      rvr_ray_plane_pool = p;
   }

   rvr_ray_plane *p = rvr_ray_plane_pool;
   rvr_ray_plane_pool = p->next;
   p->next = NULL;

   return p;
}

static void rvr_ray_plane_free(rvr_ray_plane *pl)
{
   if(pl==NULL)
      return;

   //Find last
   rvr_ray_plane *last = pl;
   while(last->next!=NULL)
      last = last->next;

   last->next = rvr_ray_plane_pool;
   rvr_ray_plane_pool = pl;
}

static RvR_ray_depth_buffer_entry *rvr_ray_depth_buffer_entry_new()
{
   if(rvr_ray_depth_buffer_entry_pool==NULL)
   {
      RvR_ray_depth_buffer_entry *e = RvR_malloc(sizeof(*e) * 256);
      memset(e, 0, sizeof(*e) * 256);

      for(int i = 0; i<255; i++)
         e[i].next = &e[i + 1];
      rvr_ray_depth_buffer_entry_pool = e;
   }

   RvR_ray_depth_buffer_entry *e = rvr_ray_depth_buffer_entry_pool;
   rvr_ray_depth_buffer_entry_pool = e->next;
   e->next = NULL;

   return e;
}

static void rvr_ray_depth_buffer_entry_free(RvR_ray_depth_buffer_entry *ent)
{
   if(ent==NULL)
      return;

   //Find last
   RvR_ray_depth_buffer_entry *last = ent;
   while(last->next!=NULL)
      last = last->next;

   last->next = rvr_ray_depth_buffer_entry_pool;
   rvr_ray_depth_buffer_entry_pool = ent;
}

#endif
#endif
