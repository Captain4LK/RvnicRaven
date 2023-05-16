/*
RvnicRaven - raycast drawing

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
   uint16_t start[RVR_XRES_MAX+2];
   uint16_t end[RVR_XRES_MAX+2];

   ray_plane *next;
};

typedef struct
{
   uint32_t flags;
   uint16_t texture;
   RvR_fix16 depth_sort;

   union
   {
      struct
      {
         //Camera space coordinates
         RvR_fix16 wx;
         RvR_fix16 wy;
      }bill;
      struct
      {
         RvR_fix16 dir;
         RvR_fix16 u0;
         RvR_fix16 u1;

         RvR_fix16 x0;
         RvR_fix16 y0;
         RvR_fix16 z0;
         RvR_fix16 x1;
         RvR_fix16 y1;
         RvR_fix16 z1;

         //Camera space coordinates
         RvR_fix16 wx0;
         RvR_fix16 wy0;
         RvR_fix16 wx1;
         RvR_fix16 wy1;
      }wall;
      struct
      {
         RvR_fix16 x0;
         RvR_fix16 z0;
         RvR_fix16 x1;
         RvR_fix16 z1;
         RvR_fix16 x2;
         RvR_fix16 z2;
         RvR_fix16 x3;
         RvR_fix16 z3;


         //Depth of center point
         RvR_fix16 depth;
         RvR_fix16 sx;

         //Camera space coordinates
         RvR_fix16 wx;
         RvR_fix16 wy;
      }floor;
   }as;
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   RvR_fix16 dir;

   RvR_fix16 z_min;
   RvR_fix16 z_max;
}ray_sprite;
//-------------------------------------

//Variables
struct
{
   RvR_ray_depth_buffer_entry *floor[RVR_XRES_MAX];
   RvR_ray_depth_buffer_entry *ceiling[RVR_XRES_MAX];
}ray_depth_buffer;

static ray_sprite *ray_sprites = NULL;

static RvR_fix16 ray_span_start[RVR_YRES_MAX] = {0};

static ray_plane *ray_planes[128] = {0};
static ray_plane *ray_plane_pool = NULL;
static RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_pool = NULL;
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

static void ray_sprite_draw_billboard(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp);
static void ray_sprite_draw_wall(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp);
static void ray_sprite_draw_floor(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp);
static void ray_floor_span_draw(const RvR_ray_cam *cam, const ray_sprite *sp, int x0, int x1, int y, const RvR_texture *texture);
static int ray_sprite_comp(const void *a, const void *b);
static int ray_sprite_can_back(const ray_sprite *a, const ray_sprite *b, const RvR_ray_cam *cam);
static int ray_wsprite_can_back(const ray_sprite * restrict a, const ray_sprite * restrict b);
//-------------------------------------

//Function implementations

void RvR_ray_draw_begin()
{
   //Clear depth buffer
   for(int i = 0;i<RvR_xres();i++)
   {
      ray_depth_buffer_entry_free(ray_depth_buffer.floor[i]);
      ray_depth_buffer_entry_free(ray_depth_buffer.ceiling[i]);

      ray_depth_buffer.floor[i] = NULL;
      ray_depth_buffer.ceiling[i] = NULL;
   }

   //Clear planes
   for(int i = 0;i<128;i++)
   {
      ray_plane_free(ray_planes[i]);
      ray_planes[i] = NULL;
   }

   RvR_array_length_set(ray_sprites,0);
}

void RvR_ray_draw_end(const RvR_ray_cam *cam, const RvR_ray_map *map)
{
   qsort(ray_sprites,RvR_array_length(ray_sprites),sizeof(*ray_sprites),ray_sprite_comp);
   int len = RvR_array_length(ray_sprites);
   for(int i = 0;i<len;i++)
   {
      int swaps = 0;
      int j = i+1;
      while(j<len)
      {
         if(ray_sprite_can_back(ray_sprites+i,ray_sprites+j,cam))
         {
            j++;
         }
         else if(i+swaps>j)
         {
            //This case usually happens when walls intersect
            //Here we would split the wall, 
            //but since intersecting walls aren't supported we just pretend nothing happended
            j++;
         }
         else
         {
            ray_sprite tmp = ray_sprites[j];
            for(int w = j;w>i;w--)
               ray_sprites[w] = ray_sprites[w-1];
            ray_sprites[i] = tmp;
            j = i+1;
            swaps++;
         }
      }
   }

   for(int i = 0;i<RvR_array_length(ray_sprites);i++)
   {
      ray_sprite *sp = ray_sprites+i;
      if(sp->flags&8)
         ray_sprite_draw_wall(cam,map,sp);
      else if(sp->flags&16)
         ray_sprite_draw_floor(cam,map,sp);
      else
         ray_sprite_draw_billboard(cam,map,sp);
   }
}

RvR_ray_pixel_info RvR_ray_map_to_screen(const RvR_ray_cam *cam, RvR_fix16 x, RvR_fix16 y, RvR_fix16 z)
{
   RvR_ray_pixel_info result;

   RvR_fix16 to_pointx = x-cam->x;
   RvR_fix16 to_pointy = y-cam->y;
   RvR_fix16 cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);

   RvR_fix16 middle_column = RvR_xres()/2;

   //rotate the point to camera space (y left/right, x forw/backw)
   RvR_fix16 tmp = to_pointx;
   to_pointx = RvR_fix16_mul(to_pointx,cos)+RvR_fix16_mul(to_pointy,sin);
   to_pointy = RvR_fix16_mul(tmp,sin)+RvR_fix16_mul(to_pointy,cos);

   result.depth = to_pointx;

   result.x = middle_column-(to_pointy*middle_column)/RvR_non_zero(RvR_abs(RvR_fix16_mul(fovx,result.depth)));
   result.y = ((z-cam->z)*RvR_yres())/RvR_non_zero(RvR_abs(RvR_fix16_mul(fovy,result.depth)));
   result.y = RvR_yres()/2-result.y+cam->shear;

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
   RvR_fix16 dir0x = RvR_fix16_cos(cam->dir-(cam->fov/2));
   RvR_fix16 dir0y = RvR_fix16_sin(cam->dir-(cam->fov/2));
   RvR_fix16 dir1x = RvR_fix16_cos(cam->dir+(cam->fov/2));
   RvR_fix16 dir1y = RvR_fix16_sin(cam->dir+(cam->fov/2));
   RvR_fix16 cos = RvR_non_zero(RvR_fix16_cos(cam->fov/2));
   dir0x = RvR_fix16_div(dir0x,cos);
   dir0y = RvR_fix16_div(dir0y,cos);
   dir1x = RvR_fix16_div(dir1x,cos);
   dir1y = RvR_fix16_div(dir1y,cos);

   RvR_fix16 dx = dir1x-dir0x;
   RvR_fix16 dy = dir1y-dir0y;
   RvR_fix16 current_dx = 0;
   RvR_fix16 current_dy = 0;

   RvR_ray_hit_result hits[RVR_RAY_MAX_STEPS] = {0};
   uint16_t hit_count = 0;

   for(int16_t x = 0;x<RvR_xres();x++)
   {
      //Here by linearly interpolating the direction vector its length changes,
      //which in result achieves correcting the fish eye effect (computing
      //perpendicular distance).
      RvR_ray r;
      r.x = cam->x;
      r.y = cam->y;
      r.dirx = dir0x+(current_dx/(RvR_xres()-1));
      r.diry = dir0y+(current_dy/(RvR_xres()-1));
      current_dx+=dx;
      current_dy+=dy;

      hit_count = 0;

      RvR_fix16 current_posx = r.x;
      RvR_fix16 current_posy = r.y;
      RvR_fix16 current_squarex = r.x/65536;
      RvR_fix16 current_squarey = r.y/65536;

      RvR_fix16 old_floor = INT32_MIN;
      RvR_fix16 old_ceiling = INT32_MIN;
      uint16_t old_ftex = UINT16_MAX;
      uint16_t old_ctex = UINT16_MAX;
      RvR_fix16 floor = old_floor;
      RvR_fix16 ceiling = old_ceiling;
      uint16_t ftex = old_ftex;
      uint16_t ctex = old_ctex;

      RvR_fix16 deltax = RvR_abs(RvR_fix16_div(65536,RvR_non_zero(r.dirx)));
      RvR_fix16 deltay = RvR_abs(RvR_fix16_div(65536,RvR_non_zero(r.diry)));

      RvR_fix16 stepx;
      RvR_fix16 stepy;
      RvR_fix16 side_distx;
      RvR_fix16 side_disty;
      int side = 0;

      if(r.dirx<0)
      {
         stepx = -1;
         side_distx = RvR_fix16_mul(r.x-current_squarex*65536,deltax);
      }
      else
      {
         stepx = 1;
         side_distx = RvR_fix16_mul(current_squarex*65536+65536-r.x,deltax);
      }

      if(r.diry<0)
      {
         stepy = -1;
         side_disty = RvR_fix16_mul(r.y-current_squarey*65536,deltay);
      }
      else
      {
         stepy = 1;
         side_disty = RvR_fix16_mul(current_squarey*65536+65536-r.y,deltay);
      }

      for(int i = 0;i<RVR_RAY_MAX_STEPS;i++)
      {
         // DDA step
         if(side_distx<side_disty)
         {
            side_distx+=deltax;
            current_squarex+=stepx;
            side = 0;
         }
         else
         {
            side_disty+=deltay;
            current_squarey+=stepy;
            side = 1;
         }

         int inbounds = RvR_ray_map_inbounds(map,current_squarex,current_squarey);
         if(inbounds)
         {
            floor = RvR_ray_map_floor_height_at_us(map,current_squarex,current_squarey);
            ceiling = RvR_ray_map_ceiling_height_at_us(map,current_squarex,current_squarey);
            ftex = RvR_ray_map_floor_tex_at_us(map,current_squarex,current_squarey);
            ctex = RvR_ray_map_ceil_tex_at_us(map,current_squarex,current_squarey);
         }

         if(!inbounds||i==RVR_RAY_MAX_STEPS-1||floor!=old_floor||ceiling!=old_ceiling||ftex!=old_ftex||ctex!=old_ctex)
         {
            RvR_ray_hit_result h;
            h.posx = current_posx;
            h.posy = current_posy;
            h.squarex = current_squarex;
            h.squarey = current_squarey;

            if(!side)
            {
               h.distance = (side_distx-deltax);
               h.posy = r.y+RvR_fix16_mul(h.distance,r.diry);
               h.posx = current_squarex*65536;
               h.direction = 3;
               if(stepx==-1)
               {
                  h.direction = 1;
                  h.posx+=65536;
               }
            }
            else
            {
               h.distance = (side_disty-deltay);
               h.posx = r.x+RvR_fix16_mul(h.distance,r.dirx);
               h.posy = current_squarey*65536;
               h.direction = 2;
               if(stepy==-1)
               {
                  h.direction = 0;
                  h.posy+=65536;
               }
            }
            
            if(RvR_ray_map_inbounds(map,current_squarex,current_squarey))
            {
               h.wall_ftex = RvR_ray_map_wall_ftex_at_us(map,current_squarex,current_squarey);
               h.wall_ctex = RvR_ray_map_wall_ctex_at_us(map,current_squarex,current_squarey);
            }
            else
            {
               h.wall_ftex = map->sky_tex;
               h.wall_ctex = map->sky_tex;
            }

            h.fheight = 0;
            h.cheight = (127*65536)/8;
            h.floor_tex = map->sky_tex;
            h.ceil_tex = map->sky_tex;

            switch(h.direction)
            {
            case 0:
               h.texture_coord = (h.posx)&65535;
               if(RvR_ray_map_inbounds(map,current_squarex,current_squarey+1))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex,current_squarey+1);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex,current_squarey+1);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex,h.squarey+1);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex,h.squarey+1);
               }
               break;
            case 1:
               h.texture_coord = (-h.posy)&65535;
               if(RvR_ray_map_inbounds(map,current_squarex+1,current_squarey))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex+1,current_squarey);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex+1,current_squarey);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex+1,h.squarey);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex+1,h.squarey);
               }
               break;
            case 2:
               h.texture_coord = (-h.posx)&65535;
               if(RvR_ray_map_inbounds(map,current_squarex,current_squarey-1))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex,current_squarey-1);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex,current_squarey-1);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex,h.squarey-1);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex,h.squarey-1);
               }
               break;
            case 3:
               h.texture_coord = (h.posy)&65535;
               if(RvR_ray_map_inbounds(map,current_squarex-1,current_squarey))
               {
                  h.fheight = RvR_ray_map_floor_height_at_us(map,current_squarex-1,current_squarey);
                  h.cheight = RvR_ray_map_ceiling_height_at_us(map,current_squarex-1,current_squarey);
                  h.floor_tex = RvR_ray_map_floor_tex_at_us(map,h.squarex-1,h.squarey);
                  h.ceil_tex = RvR_ray_map_ceil_tex_at_us(map,h.squarex-1,h.squarey);
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
      
      ray_draw_column(map,cam,hits,hit_count,x,r);

   }

   //Render floor planes
   RvR_fix16 middle_row = (RvR_yres()/2)+cam->shear;
   for(int i = 0;i<128;i++)
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
            int skyw = 1<<RvR_log2(texture->width);
            int skyh = 1<<RvR_log2(texture->height);
            int mask = skyh-1;

            RvR_fix16 angle_step = (skyw*65536)/RvR_xres();
            RvR_fix16 tex_step = (65536*skyh-1)/RvR_yres();

            RvR_fix16 angle = (cam->dir)*1024;
            angle+=(pl->min-1)*angle_step;

            for(int x = pl->min;x<pl->max+1;x++)
            {
               //Sky is rendered fullbright, no lut needed
               uint8_t * restrict pix = &RvR_framebuffer()[(pl->start[x])*RvR_xres()+x-1];
               const uint8_t * restrict tex = &texture->data[((angle>>16)&(skyw-1))*skyh];
               const uint8_t * restrict col = RvR_shade_table(32);

               //Split in two parts: above and below horizon
               int middle = RvR_max(0,RvR_min(RvR_yres(),middle_row+RvR_yres()/32));
               int tex_start = pl->start[x];
               int tex_end = middle;
               if(tex_end>pl->end[x])
                  tex_end = pl->end[x];
               if(tex_start>tex_end)
                  tex_end = tex_start;
               if(tex_start>middle)
                  tex_end = tex_start-1;
               int solid_end = pl->end[x];
               RvR_fix16 texture_coord = (RvR_yres()-middle+pl->start[x])*tex_step;

               for(int y = tex_start;y<tex_end+1;y++)
               {
                  *pix = tex[texture_coord>>16];
                  texture_coord+=tex_step;
                  pix+=RvR_xres();
               }
               RvR_fix16 tex_coord = (RvR_yres())*tex_step-1;
               texture_coord = RvR_min(tex_coord,tex_coord-tex_step*(tex_end-middle));
               for(int y = tex_end+1;y<solid_end+1;y++)
               {
                  *pix = col[tex[(texture_coord>>16)&mask]];
                  texture_coord-=tex_step;
                  pix+=RvR_xres();
               }

               angle+=angle_step;
            }

            pl = pl->next;
            continue;
         }

         //Convert plane to horizontal spans
         RvR_texture *texture = RvR_texture_get(pl->tex);
         for(int x = pl->min;x<pl->max+2;x++)
         {
            RvR_fix16 s0 = pl->start[x-1];
            RvR_fix16 s1 = pl->start[x];
            RvR_fix16 e0 = pl->end[x-1];
            RvR_fix16 e1 = pl->end[x];

            //End spans top
            for(;s0<s1&&s0<=e0;s0++)
               ray_span_draw_tex(cam,ray_span_start[s0],x-1,s0,pl->height,texture);

            //End spans bottom
            for(;e0>e1&&e0>=s0;e0--)
               ray_span_draw_tex(cam,ray_span_start[e0],x-1,e0,pl->height,texture);

            //Start spans top
            for(;s1<s0&&s1<=e1;s1++)
               ray_span_start[s1] = x-1;

            //Start spans bottom
            for(;e1>e0&&e1>=s1;e1--)
               ray_span_start[e1] = x-1;
         }

         pl = pl->next;
      }
   }
   //-------------------------------------
}

void RvR_ray_draw_sprite(const RvR_ray_cam *cam, RvR_fix16 x, RvR_fix16 y, RvR_fix16 z, RvR_fix16 dir, uint16_t sprite, uint32_t flags)
{
   ray_sprite sp = {0};
   sp.flags = flags;
   sp.texture = sprite;
   sp.x = x;
   sp.y = y;
   sp.z = z;
   sp.dir = dir;

   RvR_fix16 sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);
   RvR_fix16 sin_fov = RvR_fix16_mul(sin,fovx);
   RvR_fix16 cos_fov = RvR_fix16_mul(cos,fovx);

   //tagged as invisible
   if(flags&1)
      return;

   //Wall aligned
   if(flags&8)
   {
      RvR_texture *tex = RvR_texture_get(sprite);

      //Translate sprite to world space
      RvR_fix16 dirx = RvR_fix16_cos(dir);
      RvR_fix16 diry = RvR_fix16_sin(dir);
      RvR_fix16 half_width = (tex->width*65536)/(64*2);
      RvR_fix16 p0x = RvR_fix16_mul(-diry,half_width)+x;
      RvR_fix16 p0y = RvR_fix16_mul(dirx,half_width)+y;
      RvR_fix16 p1x = RvR_fix16_mul(diry,half_width)+x;
      RvR_fix16 p1y = RvR_fix16_mul(-dirx,half_width)+y;
      sp.x = x;
      sp.y = y;
      sp.z = z;
      sp.as.wall.dir = dir;
      sp.as.wall.u0 = 0;
      sp.as.wall.u1 = 65536*tex->width-1;

      //Translate to camera space
      RvR_fix16 x0 = p0x-cam->x;
      RvR_fix16 y0 = p0y-cam->y;
      RvR_fix16 x1 = p1x-cam->x;
      RvR_fix16 y1 = p1y-cam->y;
      RvR_fix16 tp0x = RvR_fix16_mul(-x0,sin)+RvR_fix16_mul(y0,cos);
      RvR_fix16 tp0y = RvR_fix16_mul(x0,cos_fov)+RvR_fix16_mul(y0,sin_fov);
      RvR_fix16 tp1x = RvR_fix16_mul(-x1,sin)+RvR_fix16_mul(y1,cos);
      RvR_fix16 tp1y = RvR_fix16_mul(x1,cos_fov)+RvR_fix16_mul(y1,sin_fov);

      //Behind camera
      if(tp0y<-128&&tp1y<-128)
         return;

      //Sprite not facing camera
      //--> swap p0 and p1 and toggle y-axis mirror flag
      if(RvR_fix16_mul(tp0x,tp1y)-RvR_fix16_mul(tp1x,tp0y)>0)
      {
         //One sided sprite
         if(sp.flags&128)
            return;

         RvR_fix16 tmp = tp0x;
         tp0x = tp1x;
         tp1x = tmp;

         tmp = tp0y;
         tp0y = tp1y;
         tp1y = tmp;
         sp.flags^=2;
      }

      sp.as.wall.wx0 = tp0x;
      sp.as.wall.wy0 = tp0y;
      sp.as.wall.wx1 = tp1x;
      sp.as.wall.wy1 = tp1y;

      //Here we can treat everything as if we have a 90 degree
      //fov, since the rotation to camera space transforms it to
      //that
      //Check if in fov
      //Left point in fov
      if(tp0x>=-tp0y)
      {
         //Sprite completely out of sight
         if(tp0x>tp0y)
            return;

         sp.as.wall.x0 = RvR_min(RvR_xres()*32768+RvR_fix16_div(tp0x*(RvR_xres()/2),tp0y),RvR_xres()*65536);
         sp.as.wall.z0 = tp0y;
      }
      //Left point to the left of fov
      else
      {
         //Sprite completely out of sight
         if(tp1x<-tp1y)
            return;

         sp.as.wall.x0 = 0;
         RvR_fix16 dx0 = tp1x-tp0x;
         RvR_fix16 dx1 = tp0x+tp0y;
         sp.as.wall.z0 = RvR_fix16_div(RvR_fix16_mul(dx0,dx1),tp1y-tp0y+tp1x-tp0x)-tp0x;
         sp.as.wall.u0 = sp.as.wall.u0 + RvR_fix16_div(RvR_fix16_mul(-tp0x-tp0y,sp.as.wall.u1-sp.as.wall.u0),RvR_non_zero(tp1x-tp0x+tp1y-tp0y));
      }

      //Right point in fov
      if(tp1x<=tp1y)
      {
         //sprite completely out of sight
         if(tp1x<-tp1y)
            return;

         sp.as.wall.x1 = RvR_min(RvR_xres()*32768+RvR_fix16_div(tp1x*(RvR_xres()/2),tp1y),RvR_xres()*65536);
         sp.as.wall.z1 = tp1y;
      }
      //Right point to the right of fov
      else
      {
         //sprite completely out of sight
         if(tp0x>tp0y)
            return;

         RvR_fix16 dx0 = tp1x-tp0x;
         RvR_fix16 dx1 = tp0y-tp0x;
         sp.as.wall.x1 = RvR_xres()*65536;
         sp.as.wall.z1 = tp0x-RvR_fix16_div(RvR_fix16_mul(dx0,dx1),tp1y-tp0y-tp1x+tp0x);
         sp.as.wall.u1 = RvR_fix16_div(RvR_fix16_mul(dx1,sp.as.wall.u1),RvR_non_zero(-tp1y+tp0y+tp1x-tp0x));
      }

      //Near clip sprite 
      if(sp.as.wall.z0<1024||sp.as.wall.z1<1024)
         return;

      //Far clip sprite
      if(sp.as.wall.z0>RVR_RAY_MAX_STEPS*65536&&sp.as.wall.z1>RVR_RAY_MAX_STEPS*65536)
         return;

      if(sp.as.wall.x0>sp.as.wall.x1)
         return;

      sp.as.wall.y0 = RvR_fix16_div(sp.z-cam->z,RvR_non_zero(RvR_fix16_mul(fovy,sp.as.wall.z0)));
      sp.as.wall.y0 = RvR_fix16_mul(RvR_yres()*65536,32768-sp.as.wall.y0);
      sp.as.wall.y1 = RvR_fix16_div(sp.z-cam->z,RvR_non_zero(RvR_fix16_mul(fovy,sp.as.wall.z1)));
      sp.as.wall.y1 = RvR_fix16_mul(RvR_yres()*65536,32768-sp.as.wall.y1);

      sp.depth_sort = sp.as.wall.z1;
      if(sp.as.wall.z0>sp.as.wall.z1)
         sp.depth_sort = sp.as.wall.z0;

      sp.z_min = RvR_min(sp.as.wall.z0,sp.as.wall.z1);
      sp.z_max = RvR_max(sp.as.wall.z0,sp.as.wall.z1);

      RvR_array_push(ray_sprites,sp);

      return;
   }

   //Floor alligned
   if(flags&16)
   {
      RvR_texture *tex = RvR_texture_get(sprite);

      //World space coordinates, origin at camera
      RvR_fix16 scos = RvR_fix16_cos(dir);
      RvR_fix16 ssin = RvR_fix16_sin(dir);
      RvR_fix16 half_width = (tex->width*65536)/(64*2);
      RvR_fix16 half_height = (tex->height*65536)/(64*2);
      RvR_fix16 x0 = RvR_fix16_mul(-half_width,-ssin)+RvR_fix16_mul(-half_height,scos)+x-cam->x;
      RvR_fix16 y0 = RvR_fix16_mul(-half_width,scos)+RvR_fix16_mul(-half_height,ssin)+y-cam->y;
      RvR_fix16 x1 = RvR_fix16_mul(+half_width,-ssin)+RvR_fix16_mul(-half_height,scos)+x-cam->x;
      RvR_fix16 y1 = RvR_fix16_mul(+half_width,scos)+RvR_fix16_mul(-half_height,ssin)+y-cam->y;
      RvR_fix16 x2 = RvR_fix16_mul(+half_width,-ssin)+RvR_fix16_mul(+half_height,scos)+x-cam->x;
      RvR_fix16 y2 = RvR_fix16_mul(+half_width,scos)+RvR_fix16_mul(+half_height,ssin)+y-cam->y;
      RvR_fix16 x3 = RvR_fix16_mul(-half_width,-ssin)+RvR_fix16_mul(+half_height,scos)+x-cam->x;
      RvR_fix16 y3 = RvR_fix16_mul(-half_width,scos)+RvR_fix16_mul(+half_height,ssin)+y-cam->y;

      //Move to camera space
      sp.as.floor.x0 = RvR_fix16_mul(-x0,sin)+RvR_fix16_mul(y0,cos);
      sp.as.floor.z0 = RvR_fix16_mul(x0,cos_fov)+RvR_fix16_mul(y0,sin_fov);
      sp.as.floor.x1 = RvR_fix16_mul(-x1,sin)+RvR_fix16_mul(y1,cos);
      sp.as.floor.z1 = RvR_fix16_mul(x1,cos_fov)+RvR_fix16_mul(y1,sin_fov);
      sp.as.floor.x2 = RvR_fix16_mul(-x2,sin)+RvR_fix16_mul(y2,cos);
      sp.as.floor.z2 = RvR_fix16_mul(x2,cos_fov)+RvR_fix16_mul(y2,sin_fov);
      sp.as.floor.x3 = RvR_fix16_mul(-x3,sin)+RvR_fix16_mul(y3,cos);
      sp.as.floor.z3 = RvR_fix16_mul(x3,cos_fov)+RvR_fix16_mul(y3,sin_fov);
      sp.as.floor.depth  = RvR_fix16_mul(x-cam->x,cos_fov)+RvR_fix16_mul(y-cam->y,sin_fov);

      //RvR_fix16 tpx = x-cam->x;
      //RvR_fix16 tpy = y-cam->y;
      //RvR_fix16 depth = RvR_fix16_mul(tpx,cos)+RvR_fix16_mul(tpy,sin);
      //tpx = RvR_fix16_mul(tpx,sin)-RvR_fix16_mul(tpy,cos);
      //sp.as.floor.sx = RvR_xres()*32768-RvR_fix16_div((RvR_xres()/2)*(tpx),RvR_non_zero(RvR_fix16_mul(depth,fovx)));
      sp.as.floor.wx = RvR_fix16_mul(-x-cam->x,sin)+RvR_fix16_mul(y-cam->x,cos);
      sp.as.floor.wy = RvR_fix16_mul(x-cam->x,cos_fov)+RvR_fix16_mul(y-cam->x,sin_fov);

      RvR_fix16 depth_min = RvR_min(sp.as.floor.z0,RvR_min(sp.as.floor.z1,RvR_min(sp.as.floor.z2,sp.as.floor.z3)));
      RvR_fix16 depth_max = RvR_max(sp.as.floor.z0,RvR_max(sp.as.floor.z1,RvR_max(sp.as.floor.z2,sp.as.floor.z3)));

      //Near clip
      if(depth_max<128)
         return;

      //Far clip
      if(depth_min>RVR_RAY_MAX_STEPS*65536)
         return;

      sp.depth_sort = sp.as.floor.depth;
      sp.z_min = depth_min;
      sp.z_max = depth_max;

      RvR_array_push(ray_sprites,sp);

      return;
   }

   //Billboard
   RvR_texture *tex = RvR_texture_get(sprite);

   //Project to screen
   //TODO: get rid of map_to_screen here
   RvR_ray_pixel_info p = RvR_ray_map_to_screen(cam,x,y,z);
   //sp.as.bill.x = p.x;
   //sp.as.bill.depth = p.depth;

   //Near clip
   if(p.depth<128)
      return;

   //Far clip
   if(p.depth>RVR_RAY_MAX_STEPS*65536)
      return;

   RvR_fix16 tpx = x-cam->x;
   RvR_fix16 tpy = y-cam->y;
   RvR_fix16 depth = RvR_fix16_mul(tpx,cos)+RvR_fix16_mul(tpy,sin);
   tpx = RvR_fix16_mul(tpx,sin)-RvR_fix16_mul(tpy,cos);
   RvR_fix16 x0 = RvR_xres()*32768-RvR_fix16_div((RvR_xres()/2)*(tpx+tex->width*8*64),RvR_non_zero(RvR_fix16_mul(depth,fovx)));
   RvR_fix16 x1 = RvR_xres()*32768-RvR_fix16_div((RvR_xres()/2)*(tpx-tex->width*8*64),RvR_non_zero(RvR_fix16_mul(depth,fovx)));
   sp.as.bill.wx = RvR_fix16_mul(-x-cam->x,sin)+RvR_fix16_mul(y-cam->x,cos);
   sp.as.bill.wy = RvR_fix16_mul(x-cam->x,cos_fov)+RvR_fix16_mul(y-cam->x,sin_fov);

   if(x1<0||x0>=RvR_xres()*65536)
      return;

   sp.depth_sort = p.depth;
   sp.z_min = sp.z_max = p.depth;
   RvR_array_push(ray_sprites,sp);
}

static void ray_draw_column(const RvR_ray_map *map, const RvR_ray_cam *cam, RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray)
{
   //last written Y position, can never go backwards
   RvR_fix16 f_pos_y = RvR_yres();
   RvR_fix16 c_pos_y = -1;

   RvR_fix16 middle_row = RvR_yres()/2+cam->shear;
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);

   //world coordinates (relative to camera height though)
   RvR_fix16 f_z1_world = RvR_ray_map_floor_height_at(map,cam->x/65536,cam->y/65536)-cam->z;
   RvR_fix16 c_z1_world = RvR_ray_map_ceiling_height_at(map,cam->x/65536,cam->y/65536)-cam->z;

   RvR_ray_pixel_info p = {0};
   RvR_ray_hit_result h = {0};
   p.x = x;

   //we'll be simulatenously drawing the floor and the ceiling now  
   for(RvR_fix16 j = 0;j<=hits_len;j++)
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
         RvR_fix16 wall_height = RvR_ray_map_floor_height_at(map,hit.squarex,hit.squarey);
         f_z2_world = wall_height-cam->z;
         f_z1_screen = middle_row-((f_z1_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fovy,distance)));
         f_z2_screen = middle_row-((f_z2_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fovy,distance)));

         wall_height = RvR_ray_map_ceiling_height_at(map,hit.squarex,hit.squarey);
         c_z2_world = wall_height-cam->z;
         c_z1_screen = middle_row-((c_z1_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fovy,distance)));
         c_z2_screen = middle_row-((c_z2_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fovy,distance)));
      }
      else
      {
         f_z1_screen = middle_row;
         c_z1_screen = middle_row+1;

         h = hits[j-1];
         h.distance = 65536*128;
         h.posx = RvR_fix16_mul(ray.dirx,h.distance);
         h.posy = RvR_fix16_mul(ray.diry,h.distance);

         h.direction = 0;
         h.texture_coord = 0;
         h.wall_ftex = map->sky_tex;
         h.wall_ctex = map->sky_tex;
         h.floor_tex = map->sky_tex;
         h.ceil_tex = map->sky_tex;
         h.fheight = 65536*128;
         h.cheight = 65536*128;
      }

      RvR_fix16 limit;

      //draw floor until wall
      limit_f = limit = RvR_clamp(f_z1_screen,c_pos_y+1,RvR_yres());
      if(f_pos_y>limit)
      {
         ray_plane_add(h.fheight,h.floor_tex,p.x,limit,f_pos_y-1);
         f_pos_y = limit;
      }

      //draw ceiling until wall
      limit_c = limit = RvR_clamp(c_z1_screen,-1,f_pos_y-1);
      if(limit>c_pos_y)
      {
         ray_plane_add(h.cheight,h.ceil_tex,p.x,c_pos_y+1,limit);
         c_pos_y = limit;
      }

      if(!drawing_horizon) //don't draw walls for horizon plane
      {
         p.depth = distance;
         p.depth = RvR_max(0,RvR_min(p.depth,(RVR_RAY_MAX_STEPS)*65536));

         //draw floor wall
         if(f_z1_world!=f_z2_world)
         {
            if(f_pos_y>0)  //still pixels left?
            {
               limit = ray_draw_wall(map,cam,f_pos_y,f_z1_screen,f_z2_screen,c_pos_y+1,
                                             RvR_yres(),
                                             //^ purposfully allow outside screen bounds here
                                             f_z2_world
                                             ,-1,&p,&h);
               if(f_pos_y>limit)
                  f_pos_y = limit;

               f_z1_world = f_z2_world; //for the next iteration
                           //^ purposfully allow outside screen bounds here
            }

            int limit_clip = RvR_min(limit,limit_f);
            RvR_ray_depth_buffer_entry *entry = ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip;
            entry->next = ray_depth_buffer.floor[p.x];
            ray_depth_buffer.floor[p.x] = entry;
         }

         //draw ceiling wall
         if(c_z1_world!=c_z2_world)
         {
            if(c_pos_y<RvR_yres()-1) //pixels left?
            {
               limit = ray_draw_wall(map,cam,c_pos_y,c_z1_screen,c_z2_screen,
                                 -1,f_pos_y-1,
                                 //^ puposfully allow outside screen bounds here
                                 c_z2_world 
                                 ,1,&p,&h);

               if(c_pos_y<limit)
                  c_pos_y = limit;

               c_z1_world = c_z2_world; //for the next iteration
                          //^ puposfully allow outside screen bounds here 
            }

            int limit_clip = RvR_max(limit,limit_c);
            RvR_ray_depth_buffer_entry *entry = ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip+1;
            entry->next = ray_depth_buffer.ceiling[p.x];
            ray_depth_buffer.ceiling[p.x] = entry;
         }
      }
   }
}

static int16_t ray_draw_wall(const RvR_ray_map *map, const RvR_ray_cam *cam, RvR_fix16 y_current, RvR_fix16 y_from, RvR_fix16 y_to, RvR_fix16 limit0, RvR_fix16 limit1, RvR_fix16 height, int16_t increment, RvR_ray_pixel_info *pixel_info, RvR_ray_hit_result *hit)
{
   int16_t limit = RvR_clamp(y_to,limit0,limit1);
   int start = 0;
   int end = 0;
   RvR_fix16 middle_row = RvR_yres()/2+cam->shear;
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()*65536);


   if(increment==-1)
   {
      start = limit;
      end = y_current+increment;
   }
   else if(increment==1)
   {
      start = y_current+increment;
      end = limit;
   }

   if(end<start)
      return limit;

   //Sky texture is handled differently and instead added as a plane
   if(increment==-1&&hit->wall_ftex==map->sky_tex)
   {
      ray_plane_add(hit->fheight,hit->wall_ftex,pixel_info->x,start,end);
      return limit;
   }
   else if(increment==1&&hit->wall_ctex==map->sky_tex)
   {
      ray_plane_add(hit->cheight,hit->wall_ctex,pixel_info->x,start,end);
      return limit;
   }

   RvR_texture *texture = NULL;
   //printf("wall %d\n",pixel_info->depth);
   RvR_fix16 coord_step_scaled = RvR_fix16_mul(fovy,pixel_info->depth)/RvR_yres();
   RvR_fix16 texture_coord_scaled = height+(start-middle_row+1)*coord_step_scaled;

   if(increment==-1)
      texture = RvR_texture_get(hit->wall_ftex);
   else if(increment==1)
      texture = RvR_texture_get(hit->wall_ctex);

   uint8_t * restrict pix = &RvR_framebuffer()[start*RvR_xres()+pixel_info->x];
   const uint8_t * restrict col = RvR_shade_table(RvR_max(0,RvR_min(63,(hit->direction&1)*10+(pixel_info->depth>>15))));
   const uint8_t * restrict tex = &texture->data[(hit->texture_coord>>10)*texture->height];
   RvR_fix16 y_and = (1<<RvR_log2(texture->height))-1;

#if RVR_UNROLL

   int count = end-start+1;
   int n = (count+7)/8;
   switch(count%8)
   {
   case 0: do {
           *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
   case 7: *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
   case 6: *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
   case 5: *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
   case 4: *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
   case 3: *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
   case 2: *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
   case 1: *pix = col[tex[(texture_coord_scaled>>10)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RvR_xres(); //fallthrough
           }while(--n>0);
   }

#else

   for(int i = start;i<=end;i++)
   {
      *pix = col[tex[(texture_coord_scaled>>10)&y_and]];
      texture_coord_scaled+=coord_step_scaled;
      pix+=RvR_xres();
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
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);
   RvR_fix16 middle_row = (RvR_yres()/2)+cam->shear;

   RvR_fix16 dy = middle_row-y;
   RvR_fix16 depth = RvR_fix16_div(RvR_abs(cam->z-height),RvR_non_zero(fovy));
   depth = RvR_fix16_div(depth*RvR_yres(),RvR_non_zero(RvR_abs(dy)<<16)); //TODO

   RvR_fix16 x_log = RvR_log2(texture->width);
   RvR_fix16 y_log = RvR_log2(texture->height);
   RvR_fix16 step_x = RvR_fix16_div(RvR_fix16_mul(view_sin,cam->z-height),RvR_non_zero(dy*65536));
   RvR_fix16 step_y = RvR_fix16_div(RvR_fix16_mul(view_cos,cam->z-height),RvR_non_zero(dy*65536));
   RvR_fix16 tx = cam->x+RvR_fix16_mul(view_cos,depth)+(x0-RvR_xres()/2)*step_x;
   RvR_fix16 ty = -cam->y-RvR_fix16_mul(view_sin,depth)+(x0-RvR_xres()/2)*step_y;
   RvR_fix16 x_and = (1<<x_log)-1;
   RvR_fix16 y_and = (1<<y_log)-1;

   uint8_t * restrict pix = RvR_framebuffer()+y*RvR_xres()+x0;
   const uint8_t * restrict col = RvR_shade_table(RvR_max(0,RvR_min(63,(depth>>15))));
   const uint8_t * restrict tex = texture->data;

   for(int x = x0;x<x1;x++)
   {
      uint8_t c = tex[(((tx>>10)&x_and)<<y_log)+((ty>>10)&y_and)];
      *pix = col[c];
      tx+=step_x;
      ty+=step_y;
      pix++;
   }
}

static void ray_plane_add(RvR_fix16 height, uint16_t tex, int x, int y0, int y1)
{
   x+=1;
   //Div height by 8192, since it's usually in these increments
   int hash = ((height/8192)*7+tex*3)&127;

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
         if(pl->start[x]-1==y1)
         {
            pl->start[x] = y0;
            return;
         }
         if(pl->end[x]+1==y0)
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
      pl = ray_plane_new();
      pl->next= ray_planes[hash];
      ray_planes[hash] = pl;

      pl->min = RvR_xres();
      pl->max = -1;
      pl->height = height;
      pl->tex = tex;

      //Since this is an unsigned int, we can use memset to set all values to 65535 (0xffff)
      memset(pl->start,255,sizeof(pl->start));
   }

   if(x<pl->min)
      pl->min = x;
   if(x>pl->max)
      pl->max = x;

   pl->end[x] = y1;
   pl->start[x] = y0;
}

static RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_new()
{
   if(ray_depth_buffer_entry_pool==NULL)
   {
      RvR_ray_depth_buffer_entry *e = RvR_malloc(sizeof(*e)*256,"RvR_ray ray_depth_buffer_entry pool");
      memset(e,0,sizeof(*e)*256);

      for(int i = 0;i<255;i++)
         e[i].next = &e[i+1];
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
      ray_plane *p = RvR_malloc(sizeof(*p)*16,"RvR_ray ray_plane pool");
      memset(p,0,sizeof(*p)*16);

      for(int i = 0;i<15;i++)
         p[i].next = &p[i+1];
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

static void ray_sprite_draw_billboard(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp)
{
   RvR_fix16 cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);
   RvR_fix16 sin_fov = RvR_fix16_mul(sin,fovx);
   RvR_fix16 cos_fov = RvR_fix16_mul(cos,fovx);
   RvR_fix16 middle_row = (RvR_yres()/2)+cam->shear;

   RvR_texture *texture = RvR_texture_get(sp->texture);

   RvR_fix16 tpx = sp->x-cam->x;
   RvR_fix16 tpy = sp->y-cam->y;
   RvR_fix16 depth = RvR_fix16_mul(tpx,cos_fov)+RvR_fix16_mul(tpy,sin_fov);
   tpx = RvR_fix16_mul(-tpx,sin)+RvR_fix16_mul(tpy,cos);

   //Dimensions
   RvR_fix16 top = middle_row*65536-RvR_fix16_div(RvR_yres()*(sp->z-cam->z+texture->height*1024),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
   int y0 = (top+65535)/65536;

   RvR_fix16 bot = middle_row*65536-RvR_fix16_div(RvR_yres()*(sp->z-cam->z),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
   int y1 = (bot-1)/65536;

   RvR_fix16 left = RvR_xres()*32768+RvR_fix16_div((RvR_xres()/2)*(tpx-texture->width*512),RvR_non_zero(RvR_fix16_mul(depth,fovx)));
   int x0 = (left+65535)/65536;

   RvR_fix16 right = RvR_xres()*32768+RvR_fix16_div((RvR_xres()/2)*(tpx+texture->width*512),RvR_non_zero(RvR_fix16_mul(depth,fovx)));
   int x1 = (right-1)/65536;

   //Floor and ceiling clip
   RvR_fix16 cy = middle_row*65536-RvR_fix16_div(RvR_yres()*(RvR_ray_map_floor_height_at(map,sp->x/65536,sp->y/65536)-cam->z),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
   int clip_bottom = RvR_min(cy/65536,RvR_yres());
   y1 = RvR_min(y1,clip_bottom);

   cy = middle_row*65536-RvR_fix16_div(RvR_yres()*(RvR_ray_map_ceiling_height_at(map,sp->x/65536,sp->y/65536)-cam->z),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
   int clip_top = RvR_max(cy/65536,0);
   y0 = RvR_max(y0,clip_top);

   x1 = RvR_min(x1,RvR_xres());
   RvR_fix16 step_v = RvR_fix16_mul(fovy,depth)/RvR_yres();
   RvR_fix16 step_u = RvR_fix16_mul(2*fovx,depth)/RvR_xres();
   RvR_fix16 u = RvR_fix16_mul(step_u,x0*65536-left);

   if(x0<0)
   {
      u+=(-x0)*step_u;
      x0 = 0;
      left = 0;
   }

   //Adjust for fractional part
   RvR_fix16 xfrac = left-x0*65536;
   u-=RvR_fix16_mul(xfrac,step_u);

   //Vertical flip
   if(sp->flags&4)
      step_v = -step_v;

   //Draw
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63,depth>>15));
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;
   for(int x = x0;x<x1;x++)
   {
      //Clip against walls
      int ys = y0;
      int ye = y1;

      //Clip floor
      RvR_ray_depth_buffer_entry *clip = ray_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ye>clip->limit)
            ye = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      clip = ray_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
            ys = clip->limit;
         clip = clip->next;
      }

      int tu = u/1024;
      if(sp->flags&2)
         tu = texture->width-tu-1;
      tex = &texture->data[texture->height*(tu)];
      dst = &RvR_framebuffer()[ys*RvR_xres()+x];

      RvR_fix16 v = (sp->z-cam->z)+(ys-middle_row+1)*step_v+texture->height*1024;
      if(sp->flags&4)
         v = texture->height*1024-((sp->z-cam->z)+(ys-middle_row+1)*(-step_v)+texture->height*1024);

      if(sp->flags&32)
      {
         for(int y = ys;y<ye;y++,dst+=RvR_xres())
         {
            uint8_t index = tex[(v>>10)];
            *dst = RvR_blend(col[index],*dst);
            v+=step_v;
         }
      }
      else if(sp->flags&64)
      {
         for(int y = ys;y<ye;y++,dst+=RvR_xres())
         {
            uint8_t index = tex[(v>>10)];
            *dst = RvR_blend(*dst,col[index]);
            v+=step_v;
         }
      }
      else
      {
         for(int y = ys;y<ye;y++,dst+=RvR_xres())
         {
            uint8_t index = tex[(v>>10)];
            *dst = index?col[index]:*dst;
            v+=step_v;
         }
      }

      u+=step_u;
   }
}

static void ray_sprite_draw_wall(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp)
{
   RvR_texture *texture = RvR_texture_get(sp->texture);
   RvR_fix16 scale_vertical = texture->height*1024; //texture height in map coordinates
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);
   RvR_fix16 middle_row = (RvR_yres()/2)+cam->shear;

   RvR_fix16 cy0 = RvR_fix16_div(RvR_yres()*(sp->z+scale_vertical-cam->z),RvR_fix16_mul(sp->as.wall.z0,fovy));
   RvR_fix16 cy1 = RvR_fix16_div(RvR_yres()*(sp->z+scale_vertical-cam->z),RvR_fix16_mul(sp->as.wall.z1,fovy));
   cy0 = middle_row*65536-cy0;
   cy1 = middle_row*65536-cy1;
   RvR_fix16 step_cy = RvR_fix16_div(cy1-cy0,RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
   RvR_fix16 cy = cy0;

   RvR_fix16 fy0 = RvR_fix16_div(RvR_yres()*(sp->z-cam->z),RvR_fix16_mul(sp->as.wall.z0,fovy));
   RvR_fix16 fy1 = RvR_fix16_div(RvR_yres()*(sp->z-cam->z),RvR_fix16_mul(sp->as.wall.z1,fovy));
   fy0 = middle_row*65536-fy0;
   fy1 = middle_row*65536-fy1;
   RvR_fix16 step_fy = RvR_fix16_div(fy1-fy0,RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
   RvR_fix16 fy = fy0;

   //1/z and u/z can be interpolated linearly
   //Instead of actually calculating 1/z (which would be imprecise for fixed point)
   //we bring both 1/z and u/z on the common denominator (z0*z1*w) and interpolate
   //the numerators instead
   RvR_fix16 denom = RvR_fix16_mul(sp->as.wall.x1-sp->as.wall.x0,RvR_fix16_mul(sp->as.wall.z1,sp->as.wall.z0));
   RvR_fix16 num_step_z = (sp->as.wall.z0-sp->as.wall.z1);
   RvR_fix16 num_z = RvR_fix16_mul(sp->as.wall.z1,sp->as.wall.x1-sp->as.wall.x0);

   RvR_fix16 num_step_u = (RvR_fix16_mul(sp->as.wall.z0,sp->as.wall.u1)-RvR_fix16_mul(sp->as.wall.z1,sp->as.wall.u0));
   RvR_fix16 num_u = RvR_fix16_mul(sp->as.wall.x1-sp->as.wall.x0,RvR_fix16_mul(sp->as.wall.u0,sp->as.wall.z1));

   //We don't need as much precision
   //and this prevents overflow
   //switch to i64 if you want to change this
   num_z>>=4;
   num_u>>=4;
   num_step_z>>=4;
   num_step_u>>=4;
   denom>>=4;

   //printf("%d %d\n",num_u,num_step_u);

   //Adjust for fractional part
   int x0 = (sp->as.wall.x0+65535)>>16;
   int x1 = (sp->as.wall.x1+65535)>>16;
   RvR_fix16 xfrac = sp->as.wall.x0-x0*65536;
   cy-=RvR_fix16_mul(xfrac,step_cy);
   fy-=RvR_fix16_mul(xfrac,step_fy);
   num_z-=RvR_fix16_mul(xfrac,num_step_z);
   num_u-=RvR_fix16_mul(xfrac,num_step_u);

   for(int x = x0;x<x1;x++)
   {
      RvR_fix16 depth = RvR_fix16_div(denom,RvR_non_zero(num_z));

      int y0 = (cy+65535)/65536;
      int y1 = fy/65536;

      //Clip floor
      int ybot = RvR_yres()-1;
      RvR_ray_depth_buffer_entry *clip = ray_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ybot>clip->limit)
            ybot = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      int ytop = 0;
      clip = ray_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ytop<clip->limit)
            ytop = clip->limit;
         clip = clip->next;
      }

      int wy =  ytop;
      uint8_t * restrict pix = RvR_framebuffer()+(wy*RvR_xres()+x);

      int y_to = RvR_min(y0,ybot);
      if(y_to>wy)
      {
         wy = y_to;
         pix = RvR_framebuffer()+(wy*RvR_xres()+x);
      }

      //Wall
      RvR_fix16 u = num_u/RvR_non_zero(num_z);
      //Inverting the texture coordinates directly didn't work properly,
      //so we just invert u here. 
      //TODO: investigate this
      if(sp->flags&2)
         u = texture->width-u-1;
      RvR_fix16 height = sp->z+scale_vertical-cam->z;
      RvR_fix16 coord_step_scaled = RvR_fix16_mul(fovy,depth)/RvR_yres();
      RvR_fix16 texture_coord_scaled = height+(wy-middle_row+1)*coord_step_scaled;
      //Vertical flip
      if(sp->flags&4)
      {
         coord_step_scaled = -coord_step_scaled;
         texture_coord_scaled = texture->height*1024-height+(wy-middle_row+1)*coord_step_scaled;
      }
      const uint8_t * restrict tex = &texture->data[(((uint32_t)u)%texture->width)*texture->height];
      const uint8_t * restrict col = RvR_shade_table(RvR_max(0,RvR_min(63,(depth>>15))));

      y_to = RvR_min(y1,ybot);
      if(sp->flags&32)
      {
         for(;wy<y_to;wy++)
         {
            uint8_t index = tex[(texture_coord_scaled>>10)];
            *pix = RvR_blend(col[index],*pix);
            pix+=RvR_xres();
            texture_coord_scaled+=coord_step_scaled;
         }
      }
      else if(sp->flags&64)
      {
         for(;wy<y_to;wy++)
         {
            uint8_t index = tex[(texture_coord_scaled>>10)];
            *pix = RvR_blend(*pix,col[index]);
            pix+=RvR_xres();
            texture_coord_scaled+=coord_step_scaled;
         }
      }
      else
      {
         for(;wy<y_to;wy++)
         {
            uint8_t index = tex[(texture_coord_scaled>>10)];
            *pix = index?col[index]:*pix;
            pix+=RvR_xres();
            texture_coord_scaled+=coord_step_scaled;
         }
      }

      cy+=step_cy;
      fy+=step_fy;
      num_z+=num_step_z;
      num_u+=num_step_u;
   }
}

static void ray_sprite_draw_floor(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp)
{
   //After clipping we will never have more than 8 vertices
   RvR_fix16 verts[8][3];
   RvR_fix16 verts2[8][3];
   int verts_count = 4;
   int verts2_count = 0;
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);
   RvR_texture *texture = RvR_texture_get(sp->texture);
   RvR_fix16 middle_row = (RvR_yres()/2)+cam->shear;
   RvR_fix16 middle_row2 = (RvR_yres()/2)-cam->shear;

   verts[0][0] = sp->as.floor.x0;
   verts[0][1] = sp->as.floor.z0;
   verts[0][2] = (sp->z-cam->z)*RvR_yres();
   verts[1][0] = sp->as.floor.x1;
   verts[1][1] = sp->as.floor.z1;
   verts[1][2] = (sp->z-cam->z)*RvR_yres();
   verts[2][0] = sp->as.floor.x2;
   verts[2][1] = sp->as.floor.z2;
   verts[2][2] = (sp->z-cam->z)*RvR_yres();
   verts[3][0] = sp->as.floor.x3;
   verts[3][1] = sp->as.floor.z3;
   verts[3][2] = (sp->z-cam->z)*RvR_yres();

   //Clip to view
   //-------------------------------
   //Clip left
   verts2_count = 0;
   RvR_fix16 left = verts[0][0]+verts[0][1];
   for(int i = 0;i<verts_count;i++)
   {
      int p2 = (i+1)%verts_count;
      RvR_fix16 leftn = verts[p2][0]+verts[p2][1];
      if(left>=0)
      {
         verts2[verts2_count][0] = verts[i][0];
         verts2[verts2_count][1] = verts[i][1];
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }
      if((left^leftn)<0)
      {
         verts2[verts2_count][0] = verts[i][0]+RvR_fix16_mul(RvR_fix16_div(left,left-leftn),verts[p2][0]-verts[i][0]);
         verts2[verts2_count][1] = verts[i][1]+RvR_fix16_mul(RvR_fix16_div(left,left-leftn),verts[p2][1]-verts[i][1]);
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }

      left = leftn;
   }
   if(verts2_count<=2)
      return;

   //Clip right
   verts_count = 0;
   RvR_fix16 right = verts2[0][0]-verts2[0][1];
   for(int i = 0;i<verts2_count;i++)
   {
      int p2 = (i+1)%verts2_count;
      RvR_fix16 rightn = verts2[p2][0]-verts2[p2][1];
      if(right<=0)
      {
         verts[verts_count][0] = verts2[i][0];
         verts[verts_count][1] = verts2[i][1];
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }
      if((right^rightn)<0)
      {
         verts[verts_count][0] = verts2[i][0]+RvR_fix16_mul(RvR_fix16_div(right,right-rightn),verts2[p2][0]-verts2[i][0]);
         verts[verts_count][1] = verts2[i][1]+RvR_fix16_mul(RvR_fix16_div(right,right-rightn),verts2[p2][1]-verts2[i][1]);
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }

      right = rightn;
   }
   if(verts_count<=2)
      return;

   //Clip bottom
   verts2_count = 0;
   RvR_fix16 down = verts[0][2]+RvR_fix16_mul(verts[0][1],fovy)*middle_row2;
   for(int i = 0;i<verts_count;i++)
   {
      int p2 = (i+1)%verts_count;
      RvR_fix16 downn = verts[p2][2]+RvR_fix16_mul(verts[p2][1],fovy)*middle_row2;

      if(down>=0)
      {
         verts2[verts2_count][0] = verts[i][0];
         verts2[verts2_count][1] = verts[i][1];
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }
      if((down^downn)<0)
      {
         verts2[verts2_count][0] = verts[i][0]+RvR_fix16_div(RvR_fix16_mul(down,verts[p2][0]-verts[i][0]),down-downn);
         verts2[verts2_count][1] = verts[i][1]+RvR_fix16_div(RvR_fix16_mul(down,verts[p2][1]-verts[i][1]),down-downn);
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }

      down = downn;
   }
   if(verts2_count<=2)
      return;

   //Clip top
   verts_count = 0;
   RvR_fix16 up = verts2[0][2]+RvR_fix16_mul(verts2[0][1],fovy)*(middle_row2-RvR_yres());
   for(int i = 0;i<verts2_count;i++)
   {
      int p2 = (i+1)%verts2_count;
      RvR_fix16 upn = verts2[p2][2]+RvR_fix16_mul(verts2[p2][1],fovy)*(middle_row2-RvR_yres());

      if(up<=0)
      {
         verts[verts_count][0] = verts2[i][0];
         verts[verts_count][1] = verts2[i][1];
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }
      if((up^upn)<0)
      {
         verts[verts_count][0] = verts2[i][0]+RvR_fix16_mul(RvR_fix16_div(up,RvR_non_zero(up-upn)),verts2[p2][0]-verts2[i][0]);
         verts[verts_count][1] = verts2[i][1]+RvR_fix16_mul(RvR_fix16_div(up,RvR_non_zero(up-upn)),verts2[p2][1]-verts2[i][1]);
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }

      up = upn;
   }
   if(verts_count<=2)
      return;

   //Project to screen
   for(int i = 0;i<verts_count;i++)
   {
      verts[i][0] = RvR_max(0,RvR_min(RvR_xres()*65536-1,RvR_xres()*32768+RvR_fix16_div(verts[i][0]*(RvR_xres()/2),RvR_non_zero(verts[i][1]))));
      verts[i][2] = RvR_max(0,RvR_min(RvR_yres()*65536-1,middle_row*65536-RvR_fix16_div(verts[i][2],RvR_non_zero(RvR_fix16_mul(verts[i][1],fovy)))));
   }
   //-------------------------------

   //Rasterize
   //-------------------------------
   int index_minl = 0;
   RvR_fix16 xmin = RvR_xres()*65536;
   for(int i = 0;i<verts_count;i++)
   {
      if(verts[i][0]<xmin)
      {
         xmin = verts[i][0];
         index_minl = i;
      }
   }

   int index_minr = index_minl;

   RvR_fix16 le_y = 0;
   RvR_fix16 le_dy = 0;
   RvR_fix16 le_width = 0;
   RvR_fix16 re_y = 0;
   RvR_fix16 re_dy = 0;
   RvR_fix16 re_width = 0;

   int x = (xmin+65535)/65536;

   int prev_start = RvR_yres();
   int prev_end = 0;
   for(;;)
   {
      if(!le_width)
      {
         int index_rightl = (index_minl-1+verts_count)%verts_count;
         le_width = (verts[index_rightl][0]+65535)/65536-(verts[index_minl][0]+65535)/65536;
         if(le_width<0)
            break;

         le_dy = RvR_fix16_div(verts[index_rightl][2]-verts[index_minl][2],RvR_non_zero(verts[index_rightl][0]-verts[index_minl][0]));
         le_y = verts[index_minl][2]+RvR_fix16_mul(le_dy,((verts[index_minl][0]+65535)/65536)*65536-verts[index_minl][0]);

         index_minl = index_rightl;
      }

      if(!re_width)
      {
         int index_rightr = (index_minr+1)%verts_count;
         re_width = (verts[index_rightr][0]+65535)/65536-(verts[index_minr][0]+65535)/65536;
         if(re_width<0)
            break;

         re_dy = RvR_fix16_div(verts[index_rightr][2]-verts[index_minr][2],RvR_non_zero(verts[index_rightr][0]-verts[index_minr][0]));
         re_y = verts[index_minr][2]+RvR_fix16_mul(re_dy,((verts[index_minr][0]+65535)/65536)*65536-verts[index_minr][0]);

         index_minr = index_rightr;
      }

      if(!re_width&&!le_width)
         break;

      int width = RvR_min(le_width,re_width);

      le_width-=width;
      re_width-=width;

      while(width-->0)
      {
         int start = RvR_max(0,RvR_min(RvR_yres()-1,(le_y+65535)/65536));
         int end = RvR_max(0,RvR_min(RvR_yres()-1,(re_y+65535)/65536));
         if(start>end)
         {
            int tmp = start;
            start = end;
            end = tmp;
         }

         //We just clip to the middle of the sprite,
         //it's fine since floor aligned sprites
         //aren't supposted to intersect walls
         //Clip floor
         RvR_ray_depth_buffer_entry *clip = ray_depth_buffer.floor[x];
         while(clip!=NULL)
         {
            if(sp->as.floor.depth>clip->depth&&end>clip->limit)
               end = clip->limit;
            clip = clip->next;
         }

         //Clip ceiling
         clip = ray_depth_buffer.ceiling[x];
         while(clip!=NULL)
         {
            if(sp->as.floor.depth>clip->depth&&start<clip->limit)
               start = clip->limit;
            clip = clip->next;
         }

         RvR_fix16 s0 = prev_start;
         RvR_fix16 s1 = start;
         RvR_fix16 e0 = prev_end;
         RvR_fix16 e1 = end;

         //End spans top
         for(;s0<s1&&s0<=e0;s0++)
            ray_floor_span_draw(cam,sp,ray_span_start[s0],x,s0,texture);

         //End spans bottom
         for(;e0>e1&&e0>=s0;e0--)
            ray_floor_span_draw(cam,sp,ray_span_start[e0],x,e0,texture);

         //Start spans top
         for(;s1<s0&&s1<=e1;s1++)
            ray_span_start[s1] = x;

         //Start spans bottom
         for(;e1>e0&&e1>=s1;e1--)
            ray_span_start[e1] = x;

         prev_start = start;
         prev_end = end;
         le_y+=le_dy;
         re_y+=re_dy;
         x++;
      }
   }

   //End remaining
   RvR_fix16 s0 = prev_start;
   RvR_fix16 s1 = prev_end;
   for(;s0<=s1;s0++)
      ray_floor_span_draw(cam,sp,ray_span_start[s0],x,s0,texture);
   //-------------------------------
}

static void ray_floor_span_draw(const RvR_ray_cam *cam, const ray_sprite *sp, int x0, int x1, int y, const RvR_texture *texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   if(texture==NULL)
      return;

   RvR_fix16 view_sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 view_cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);
   RvR_fix16 middle_row = (RvR_yres()/2)+cam->shear;

   RvR_fix16 dy = middle_row-y;
   RvR_fix16 depth = RvR_fix16_div(RvR_abs(cam->z-sp->z),RvR_non_zero(fovy));
   depth = RvR_fix16_div(depth*RvR_yres(),RvR_non_zero(RvR_abs(dy)<<16)); //TODO

   RvR_fix16 x_log = RvR_log2(texture->width);
   RvR_fix16 y_log = RvR_log2(texture->height);
   RvR_fix16 step_x = RvR_fix16_div(RvR_fix16_mul(view_sin,cam->z-sp->z),RvR_non_zero(dy*65536));
   RvR_fix16 step_y = RvR_fix16_div(RvR_fix16_mul(view_cos,cam->z-sp->z),RvR_non_zero(dy*65536));
   RvR_fix16 tx = (cam->x-sp->x+texture->width*0)+RvR_fix16_mul(view_cos,depth)+(x0-RvR_xres()/2)*step_x;
   RvR_fix16 ty = -(cam->y-sp->y+texture->height*0)-RvR_fix16_mul(view_sin,depth)+(x0-RvR_xres()/2)*step_y;
   RvR_fix16 x_and = (1<<x_log)-1;
   RvR_fix16 y_and = (1<<y_log)-1;

   //Rotate texture coordinates according to sprite rotation
   RvR_fix16 sp_sin = RvR_fix16_sin(-sp->dir);
   RvR_fix16 sp_cos = RvR_fix16_cos(-sp->dir);
   RvR_fix16 tmp = tx;
   tx = RvR_fix16_mul(-sp_sin,tx)+RvR_fix16_mul(sp_cos,ty);
   ty = RvR_fix16_mul(sp_cos,tmp)+RvR_fix16_mul(sp_sin,ty);
   tmp = step_x;
   step_x = RvR_fix16_mul(-sp_sin,step_x)+RvR_fix16_mul(sp_cos,step_y);
   step_y = RvR_fix16_mul(sp_cos,tmp)+RvR_fix16_mul(sp_sin,step_y);

   //Offset texture coordinates
   //since sprites are anchored in their middle
   tx+=texture->width*512;
   ty+=texture->height*512;

   uint8_t * restrict pix = RvR_framebuffer()+y*RvR_xres()+x0;
   const uint8_t * restrict col = RvR_shade_table(RvR_max(0,RvR_min(63,(depth>>15))));
   const uint8_t * restrict tex = texture->data;

   if(sp->flags&32)
   {
      for(int x = x0;x<x1;x++)
      {
         uint8_t c = tex[(((tx>>10)&x_and)<<y_log)+((ty>>10)&y_and)];
         *pix = RvR_blend(col[c],*pix);
         tx+=step_x;
         ty+=step_y;
         pix++;
      }
   }
   else if(sp->flags&64)
   {
      for(int x = x0;x<x1;x++)
      {
         uint8_t c = tex[(((tx>>10)&x_and)<<y_log)+((ty>>10)&y_and)];
         *pix = RvR_blend(*pix,col[c]);
         tx+=step_x;
         ty+=step_y;
         pix++;
      }
   }
   else
   {
      for(int x = x0;x<x1;x++)
      {
         uint8_t c = tex[(((tx>>10)&x_and)<<y_log)+((ty>>10)&y_and)];
         *pix = c?col[c]:*pix;
         tx+=step_x;
         ty+=step_y;
         pix++;
      }
   }
}

static int ray_sprite_comp(const void *a, const void *b)
{
   const ray_sprite *sa = a;
   const ray_sprite *sb = b;

   return sb->depth_sort-sa->depth_sort;
}

static int ray_sprite_can_back(const ray_sprite *a, const ray_sprite *b, const RvR_ray_cam *cam)
{
   //Separate cases:
   //wall - wall : full check
   //sprite - sprite: nothing, only depth compare?
   //wall - sprite: partial check, only perp dot

   //Wall - Wall check is put in
   //a separate function
   if(a->flags&8&&b->flags&8)
      return ray_wsprite_can_back(a,b);

   //Sprite - Sprite check is
   //a lot simpler
   if(!(a->flags&8)&&!(b->flags&8))
   {
      //If one is floor sprite, check height
      if(a->flags&16||b->flags&16)
      {
         //a completely behind b
         //We only want to sort them by height if
         //they overlap on the z-axis
         if(a->z_min>b->z_max)
            return 1;

         //If one of the sprites is above the camera, the higher
         //ones needs to be drawn first
         if(a->z>cam->z||b->z>cam->z)
            return (a->z)>=(b->z);
         //Otherwise, the lower one get's drawn first
         return (a->z)<=(b->z);
      }

      //Due to the z sorting before, the sprites
      //are already sorted by z.
      return 1;
   }

   int64_t x00 = 0;
   int64_t z00 = 0;
   int64_t x01 = 0;
   int64_t z01 = 0;
   int64_t x10 = 0;
   int64_t z10 = 0;
   int64_t x11 = 0;
   int64_t z11 = 0;
   if(a->flags&8)
   {
      x00 = a->as.wall.wx0;
      x01 = a->as.wall.wx1;
      z00 = a->as.wall.wy0;
      z01 = a->as.wall.wy1;
   }
   else if(a->flags&16)
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

   if(b->flags&8)
   {
      x10 = b->as.wall.wx0;
      x11 = b->as.wall.wx1;
      z10 = b->as.wall.wy0;
      z11 = b->as.wall.wy1;
   }
   else if(b->flags&16)
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
   if(a->flags&8)
   {
      int64_t dx0 = x01-x00;
      int64_t dz0 = z01-z00;
      int64_t cross00 = dx0*(z10-z00)-dz0*(x10-x00);

      //sprite b in front wall a
      if(cross00<=0)
         return 1;
   }
   else
   {
      int64_t dx1 = x11-x10;
      int64_t dz1 = z11-z10;
      int64_t cross10 = dx1*(z00-z10)-dz1*(x00-x10);

      //sprite a behind wall b
      if(cross10>=0)
         return 1;
   }

   return 0;
}

static int ray_wsprite_can_back(const ray_sprite * restrict a, const ray_sprite * restrict b)
{
   //NOTE(Captain4LK): We use 64 bit precision here, 
   //you could do it without, but I haven't
   //really checked how well that works,
   //especially with very long wall sprites
   int64_t x00 = a->as.wall.wx0;
   int64_t x01 = a->as.wall.wx1;
   int64_t z00 = a->as.wall.wy0;
   int64_t z01 = a->as.wall.wy1;

   int64_t x10 = b->as.wall.wx0;
   int64_t x11 = b->as.wall.wx1;
   int64_t z10 = b->as.wall.wy0;
   int64_t z11 = b->as.wall.wy1;

   //a completely behind b
   if(a->z_min>b->z_max)
      return 1;

   //No screen x overlap
   if(a->as.wall.x0>=b->as.wall.x1||a->as.wall.x1<=b->as.wall.x0)
      return 1;

   int64_t dx0 = x01-x00;
   int64_t dz0 = z01-z00;
   int64_t dx1 = x11-x10;
   int64_t dz1 = z11-z10;

   //2d cross product/perp dot product
   int64_t cross00 = dx0*(z10-z00)-dz0*(x10-x00);
   int64_t cross01 = dx0*(z11-z00)-dz0*(x11-x00);
   int64_t cross10 = dx1*(z00-z10)-dz1*(x00-x10);
   int64_t cross11 = dx1*(z01-z10)-dz1*(x01-x10);

   //All points of b in front of a
   if(cross00<=0&&cross01<=0)
      return 1;

   //All points of a behind b
   if(cross10>=0&&cross11>=0)
      return 1;

   //Need swapping
   return 0;
}
//-------------------------------------
