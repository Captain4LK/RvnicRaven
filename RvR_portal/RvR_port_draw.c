/*
RvnicRaven - portal drawing

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
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   RvR_fix22 x0;
   RvR_fix22 xw0;
   RvR_fix22 z0;
   RvR_fix22 zw0;
   RvR_fix22 x1;
   RvR_fix22 xw1;
   RvR_fix22 z1;
   RvR_fix22 zw1;
   RvR_fix22 zfront;

   RvR_fix22 u0;
   RvR_fix22 u1;

   uint16_t wall;
   uint16_t sector;
}RvR_port_dwall;

typedef struct port_plane port_plane;
struct port_plane
{
   int32_t min;
   int32_t max;
   uint16_t sector;
   uint8_t where; //0 --> ceiling; 1 --> floor
   uint16_t start[RVR_XRES_MAX+2];
   uint16_t end[RVR_XRES_MAX+2];

   port_plane *next;
};

typedef struct
{
   uint32_t flags;
   int16_t sector;
   uint16_t texture;
   void *ref;

   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
   RvR_fix22 dir;

   RvR_fix22 z_min;
   RvR_fix22 z_max;

   union
   {
      struct
      {
         //Camera space coordinates
         //wy is depth
         RvR_fix22 wx;
         RvR_fix22 wy;
      }bill;
      struct
      {
         RvR_fix22 u0;
         RvR_fix22 u1;

         RvR_fix22 x0;
         RvR_fix22 z0;
         RvR_fix22 x1;
         RvR_fix22 z1;

         //Camera space coordinates
         //wy is depth
         RvR_fix22 wx0;
         RvR_fix22 wy0;
         RvR_fix22 wx1;
         RvR_fix22 wy1;
      }wall;
      struct
      {
         RvR_fix22 x0;
         RvR_fix22 z0;
         RvR_fix22 x1;
         RvR_fix22 z1;
         RvR_fix22 x2;
         RvR_fix22 z2;
         RvR_fix22 x3;
         RvR_fix22 z3;

         //Camera space coordinates
         //wy is depth
         RvR_fix22 wx;
         RvR_fix22 wy;
      }floor;
   }as;
}port_sprite;
//-------------------------------------

//Variables
static int16_t *sector_stack = NULL;
static RvR_port_dwall *dwalls = NULL;

static port_plane *port_planes[128] = {0};
static port_plane *port_plane_pool = NULL;

static RvR_fix22 port_span_start[RVR_YRES_MAX] = {0};
static RvR_fix22 port_ytop[RVR_XRES_MAX] = {0};
static RvR_fix22 port_ybot[RVR_XRES_MAX] = {0};

static RvR_port_depth_buffer_entry *port_depth_buffer_entry_pool = NULL;

static const RvR_port_cam *port_cam = NULL;
static const RvR_port_map *port_map = NULL;

static port_sprite *port_sprites = NULL;

struct
{
   RvR_port_depth_buffer_entry *floor[RVR_XRES_MAX];
   RvR_port_depth_buffer_entry *ceiling[RVR_XRES_MAX];
} port_depth_buffer;
//-------------------------------------

//Function prototypes
static int dwall_can_front(const RvR_port_dwall *wa, const RvR_port_dwall *wb);

static void port_span_draw(const RvR_port_map *map, const RvR_port_cam *cam, int16_t sector, uint8_t where, int x0, int x1, int y);

static void port_plane_add(int16_t sector, uint8_t where, int x, int y0, int y1);
static port_plane *port_plane_new();
static void port_plane_free(port_plane *pl);

static RvR_port_depth_buffer_entry *port_depth_buffer_entry_new();
static void port_depth_buffer_entry_free(RvR_port_depth_buffer_entry *ent);

static int port_sprite_comp(const void *a, const void *b);
static int port_sprite_can_back(const port_sprite *a, const port_sprite *b);
static int port_wsprite_can_back(const port_sprite * restrict a, const port_sprite * restrict b);

static void port_sprite_draw_billboard(const RvR_port_map *map, const port_sprite *sp, RvR_port_selection *select);
static void port_sprite_draw_wall(const port_sprite *sp, RvR_port_selection *select);
static void port_sprite_draw_floor(const port_sprite *sp, RvR_port_selection *select);
static void port_floor_span_draw(const port_sprite *sp, int x0, int x1, int y, const RvR_texture *texture);

static void port_collect_walls(int16_t start);
//-------------------------------------

//Function implementations

void RvR_port_draw_begin(const RvR_port_map *map, const RvR_port_cam *cam)
{
   //Clear depth buffer
   for(int i = 0; i<RvR_xres(); i++)
   {
      port_depth_buffer_entry_free(port_depth_buffer.floor[i]);
      port_depth_buffer_entry_free(port_depth_buffer.ceiling[i]);

      port_depth_buffer.floor[i] = NULL;
      port_depth_buffer.ceiling[i] = NULL;
   }

   //Clear planes
   for(int i = 0; i<128; i++)
   {
      port_plane_free(port_planes[i]);
      port_planes[i] = NULL;
   }

   //Clear sector visited flags
   RvR_array_length_set(dwalls,0);
   for(int i = 0;i<map->sector_count;i++)
      map->sectors[i].visited = 0;

   //Clear sprites
   RvR_array_length_set(port_sprites, 0);

   port_cam = cam;
   port_map = map;

#if RVR_PORT_PIXELBYPIXEL
   if(RvR_key_pressed(RVR_PORT_PIXELKEY))
      RvR_render_clear(0);
#endif
}

void RvR_port_draw_map(RvR_port_selection *select)
{
   if(select!=NULL)
   {
      select->depth = INT32_MAX;
      select->type = RVR_PORT_NONE;
   }

   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov/2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres()*fovx,RvR_xres()*1024);

   for(int i = 0;i<RvR_xres();i++)
   {
      port_ytop[i] = -1;
      port_ybot[i] = RvR_yres();
   }

   int max_len = 0;

   //Start with sector of camera
   //TODO(Captain4LK): stop rendering once all port_ybot<port_ytop (?), use count_remaining for rough estimation
   port_collect_walls(port_cam->sector);
   while(RvR_array_length(dwalls)>0)
   {
      int len = (int)RvR_array_length(dwalls);
      max_len = RvR_max(max_len,len);
      //for(int i = 0;i<len;i++)
      {
         int swaps = 0;
         int j = 0+1;
         while(j<len)
         {
            if(dwall_can_front(dwalls+0,dwalls+j))
            {
               j++;
            }
            else if(0+swaps>j)
            {
               //This case usually happens when walls intersect
               //Here we would split the wall, 
               //but since intersecting walls aren't supported we just pretend nothing happended
               j++;
               //puts("skip");
            }
            else
            {
               RvR_port_dwall tmp = dwalls[j];
               for(int w = j;w>0;w--)
                  dwalls[w] = dwalls[w-1];
               dwalls[0] = tmp;
               j = 0+1;
               swaps++;
            }
         }
      }

      RvR_port_dwall tmp = dwalls[0];
      for(int w = 0;w<len-1;w++)
         dwalls[w] = dwalls[w+1];
      RvR_port_dwall *wall = &tmp;
      RvR_array_length_set(dwalls,RvR_array_length(dwalls)-1);
      int16_t sector = wall->sector;
      int16_t portal = port_map->walls[wall->wall].portal;

      //Shift texture x coordinates
      wall->u0+=((int32_t)port_map->walls[wall->wall].x_off)*1024;
      wall->u1+=((int32_t)port_map->walls[wall->wall].x_off)*1024;

      //No portal
      if(portal<0)
      {
         RvR_fix22 cy0 = RvR_fix22_div(2*RvR_yres()*(port_map->sectors[sector].ceiling-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 cy1 = RvR_fix22_div(2*RvR_yres()*(port_map->sectors[sector].ceiling-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         cy0 = 4*RvR_yres()*512-cy0;
         cy1 = 4*RvR_yres()*512-cy1;
         RvR_fix22 step_cy = RvR_fix22_div(cy1-cy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 cy = cy0;

         RvR_fix22 fy0 = RvR_fix22_div(2*RvR_yres()*(port_map->sectors[sector].floor-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 fy1 = RvR_fix22_div(2*RvR_yres()*(port_map->sectors[sector].floor-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         fy0 = 4*RvR_yres()*512-fy0;
         fy1 = 4*RvR_yres()*512-fy1;
         RvR_fix22 step_fy = RvR_fix22_div(fy1-fy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 fy = fy0;

         RvR_fix22 denom = RvR_fix22_mul(2*wall->z1,2*wall->z0);
         RvR_fix22 num_step_z = 4*(wall->z0-wall->z1);
         RvR_fix22 num_z = 4*wall->z1;

         RvR_texture *texture = RvR_texture_get(port_map->walls[wall->wall].tex_lower);
         RvR_fix22 u0 = wall->u0-(wall->u0&(~(1024*texture->width-1)));
         RvR_fix22 u1 = wall->u1-(wall->u0&(~(1024*texture->width-1)));
         int64_t num_step_u = RvR_fix22_mul(wall->z0,u1)-RvR_fix22_mul(wall->z1,u0);
         int64_t num_u = ((int64_t)u0*wall->z1)/1024;
         //int64_t num_u = RvR_fix22_mul(u0,wall->z1);

         //Adjust for fractional part
         int x0 = (wall->x0+1023)/1024;
         int x1 = (wall->x1+1023)/1024;
         RvR_fix22 xfrac = x0*1024-wall->x0;
         cy+=RvR_fix22_mul(xfrac,step_cy);
         fy+=RvR_fix22_mul(xfrac,step_fy);
         num_z+=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(wall->x1-wall->x0));
         num_u+=(xfrac*num_step_u)/RvR_non_zero(wall->x1-wall->x0);
         //num_u+=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_u),RvR_non_zero(wall->x1-wall->x0));

         for(int x = x0;x<x1;x++)
         {
            int y0 = (cy+4095)/4096;
            int y1 = fy/4096;
            RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            int64_t nu = num_u+((num_step_u*(x-x0))*1024)/RvR_non_zero(wall->x1-wall->x0);
            RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz));
            RvR_fix22 u = (RvR_fix22)((4*nu)/RvR_non_zero(nz));

            int top = port_ytop[x]+1;
            y0 = RvR_max(y0,top);
            int bottom = y0-1;
            if(bottom>=port_ybot[x])
               bottom = port_ybot[x]-1;

            if(top<=bottom)
            {
               if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
               {
                  select->type = RVR_PORT_CEILING;
                  select->as.sector = wall->sector;
                  select->depth = depth;
               }

               port_plane_add(wall->sector,0,x,top,bottom);
            }

            bottom = port_ybot[x]-1;
            y1 = RvR_min(y1,bottom);

            top = RvR_max(y1,port_ytop[x])+1;
            if(top<=bottom)
            {
               if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
               {
                  select->type = RVR_PORT_FLOOR;
                  select->as.sector = wall->sector;
                  select->depth = depth;
               }

               port_plane_add(wall->sector,1,x,top,bottom);
            }

            port_ytop[x] = RvR_yres();
            port_ybot[x] = -1;

            if(y0<=y1)
            {
               if(select!=NULL&&select->x==x&&select->y>=y0&&select->y<=y1&&select->depth>depth)
               {
                  select->type = RVR_PORT_WALL_BOT;
                  select->as.wall = wall->wall;
                  select->depth = depth;
               }

               RvR_fix22 height = port_map->sectors[wall->sector].ceiling-port_cam->z;
               RvR_fix22 coord_step_scaled = (8*fovy*depth)/RvR_yres();
               RvR_fix22 texture_coord_scaled = height*4096+(y0-RvR_yres()/2)*coord_step_scaled+((int32_t)port_map->walls[wall->wall].y_off)*65536;
               RvR_fix22 y_and = (1<<RvR_log2(texture->height))-1;
               const uint8_t * restrict tex = &texture->data[(((uint32_t)u)%texture->width)*texture->height];
               const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(depth>>9)+port_map->walls[wall->wall].shade_offset)));
               uint8_t * restrict pix = RvR_framebuffer()+(y0*RvR_xres()+x);
               for(int y = y0;y<=y1;y++)
               {
                  *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
                  pix+=RvR_xres();
                  texture_coord_scaled+=coord_step_scaled;

#if RVR_PORT_PIXELBYPIXEL
                  if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                     RvR_render_present();
#endif
               }
            }

            cy+=step_cy;
            fy+=step_fy;

            //Depth buffer
            RvR_port_depth_buffer_entry *entry = port_depth_buffer_entry_new();
            entry->depth = depth;
            entry->limit = 0;
            entry->next = port_depth_buffer.floor[x];
            port_depth_buffer.floor[x] = entry;

            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
         }
      }
      //Portal
      else
      {
         //depth values go much lower here (since you can be on the border between sectors)
         //so we use i64 here
         int64_t cy0 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t cy1 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         cy0 = RvR_yres()*512-cy0;
         cy1 = RvR_yres()*512-cy1;
         int64_t step_cy = ((cy1-cy0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t cy = cy0;

         int64_t cph0 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t cph1 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         cph0 = RvR_yres()*512-cph0;
         cph1 = RvR_yres()*512-cph1;
         int64_t step_cph = ((cph1-cph0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t cph = cph0;

         int64_t fy0 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t fy1 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         fy0 = RvR_yres()*512-fy0;
         fy1 = RvR_yres()*512-fy1;
         int64_t step_fy = ((fy1-fy0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t fy = fy0;

         int64_t fph0 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t fph1 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         fph0 = RvR_yres()*512-fph0;
         fph1 = RvR_yres()*512-fph1;
         int64_t step_fph = ((fph1-fph0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t fph = fph0;

         RvR_fix22 denom = RvR_fix22_mul(2*wall->z1,2*wall->z0);
         RvR_fix22 num_step_z = 4*(wall->z0-wall->z1);
         RvR_fix22 num_z = 4*wall->z1;

         RvR_texture *texture_lower = RvR_texture_get(port_map->walls[wall->wall].tex_lower);
         RvR_texture *texture_upper = RvR_texture_get(port_map->walls[wall->wall].tex_upper);
         RvR_fix22 u0_lower = wall->u0-(wall->u0&(~(1024*texture_lower->width-1)));
         RvR_fix22 u1_lower = wall->u1-(wall->u0&(~(1024*texture_lower->width-1)));
         RvR_fix22 u0_upper = wall->u0-(wall->u0&(~(1024*texture_upper->width-1)));
         RvR_fix22 u1_upper = wall->u1-(wall->u0&(~(1024*texture_upper->width-1)));
         RvR_fix22 num_step_u_lower = RvR_fix22_mul(wall->z0,u1_lower)-RvR_fix22_mul(wall->z1,u0_lower);
         RvR_fix22 num_u_lower = RvR_fix22_mul(u0_lower,wall->z1);
         RvR_fix22 num_step_u_upper = RvR_fix22_mul(wall->z0,u1_upper)-RvR_fix22_mul(wall->z1,u0_upper);
         RvR_fix22 num_u_upper = RvR_fix22_mul(u0_upper,wall->z1);

         //Adjust for fractional part
         int x0 = (wall->x0+1023)/1024;
         int x1 = (wall->x1+1023)/1024;
         RvR_fix22 xfrac = x0*1024-wall->x0;
         cy+=(xfrac*step_cy)/1024;
         cph+=(xfrac*step_cph)/1024;
         fy+=(xfrac*step_fy)/1024;
         fph+=(xfrac*step_fph)/1024;
         num_z+=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(wall->x1-wall->x0));
         num_u_lower+=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_u_lower),RvR_non_zero(wall->x1-wall->x0));
         num_u_upper+=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_u_upper),RvR_non_zero(wall->x1-wall->x0));

         for(int x = x0;x<x1;x++)
         {
            //TODO(Captain4LK): maybe don't cast, but keep i64?
            int y0 = (int)((cy+1023)/1024);
            int y1 = (int)(fy/1024);

            RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 nu_lower = num_u_lower+RvR_fix22_div(num_step_u_lower*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 nu_upper = num_u_upper+RvR_fix22_div(num_step_u_upper*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz));
            RvR_fix22 u_lower = (4*nu_lower)/RvR_non_zero(nz);
            RvR_fix22 u_upper = (4*nu_upper)/RvR_non_zero(nz);

            int top = port_ytop[x]+1;
            int bottom = port_ybot[x];

            y0 = RvR_max(y0,top);
            bottom = y0-1;
            if(bottom>=port_ybot[x])
               bottom = port_ybot[x]-1;

            if(top<=bottom)
            {
               if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
               {
                  select->type = RVR_PORT_CEILING;
                  select->as.sector = wall->sector;
                  select->depth = depth;
               }
               port_plane_add(wall->sector,0,x,top,bottom);
            }

            bottom = port_ybot[x]-1;
            y1 = RvR_min(y1,bottom);

            top = RvR_max(y1,port_ytop[x])+1;
            if(top<=bottom)
            {
               if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
               {
                  select->type = RVR_PORT_FLOOR;
                  select->as.sector = wall->sector;
                  select->depth = depth;
               }

               port_plane_add(wall->sector,1,x,top,bottom);
            }

            const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(depth>>9)+port_map->walls[wall->wall].shade_offset)));
            int mid = (int)(cph/1024);
            if(mid>=port_ybot[x])
               mid = port_ybot[x]-1;
            if(mid>=y0)
            {
               if(select!=NULL&&select->x==x&&select->y>=y0&&select->y<=mid&&select->depth>depth)
               {
                  select->type = RVR_PORT_WALL_TOP;
                  select->as.wall = wall->wall;
                  select->depth = depth;
               }

               RvR_fix22 height = port_map->sectors[portal].ceiling-port_cam->z;
               RvR_fix22 coord_step_scaled = (8*fovy*depth)/RvR_yres();
               RvR_fix22 texture_coord_scaled = height*4096+(y0-RvR_yres()/2)*coord_step_scaled+((int32_t)port_map->walls[wall->wall].y_off)*65536;
               const uint8_t * restrict tex = &texture_upper->data[(((uint32_t)u_upper)%texture_upper->width)*texture_upper->height];
               uint8_t * restrict pix = RvR_framebuffer()+(y0*RvR_xres()+x);
               RvR_fix22 y_and = (1<<RvR_log2(texture_upper->height))-1;
               for(int y = y0;y<=mid;y++)
               {
                  *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
                  pix+=RvR_xres();
                  texture_coord_scaled+=coord_step_scaled;

#if RVR_PORT_PIXELBYPIXEL
                  if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                     RvR_render_present();
#endif
               }
               port_ytop[x] = mid;
            }
            else
            {
               port_ytop[x] = y0-1;
            }

            mid = (int)((fph+1023)/1024);
            if(mid<=port_ytop[x])
               mid = port_ytop[x]+1;

            if(mid<=y1)
            {
               if(select!=NULL&&select->x==x&&select->y>=mid&&select->y<=y1&&select->depth>depth)
               {
                  select->type = RVR_PORT_WALL_BOT;
                  select->as.wall = wall->wall;
                  select->depth = depth;
               }

               RvR_fix22 height = port_map->sectors[portal].floor-port_cam->z;
               RvR_fix22 coord_step_scaled = (8*fovy*depth)/RvR_yres();
               RvR_fix22 texture_coord_scaled = height*4096+(mid-RvR_yres()/2)*coord_step_scaled+coord_step_scaled/2+((int32_t)port_map->walls[wall->wall].y_off)*65536;
               RvR_fix22 y_and = (1<<RvR_log2(texture_lower->height))-1;
               const uint8_t * restrict tex = &texture_lower->data[(((uint32_t)u_lower)%texture_lower->width)*texture_lower->height];
               uint8_t * restrict pix = RvR_framebuffer()+(mid*RvR_xres()+x);
               for(int y = mid;y<=y1;y++)
               {
                  *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
                  pix+=RvR_xres();
                  texture_coord_scaled+=coord_step_scaled;

#if RVR_PORT_PIXELBYPIXEL
                  if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                     RvR_render_present();
#endif
               }
               port_ybot[x] = mid;
            }
            else
            {
               port_ybot[x] = y1+1;
            }

            //Depth buffer ceiling
            RvR_port_depth_buffer_entry *entry = port_depth_buffer_entry_new();
            entry->depth = depth;
            entry->limit = port_ytop[x];
            entry->next = port_depth_buffer.ceiling[x];
            port_depth_buffer.ceiling[x] = entry;

            //Depth buffer floor
            entry = port_depth_buffer_entry_new();
            entry->depth = depth;
            entry->limit = port_ybot[x];
            entry->next = port_depth_buffer.floor[x];
            port_depth_buffer.floor[x] = entry;

            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();

            cy+=step_cy;
            cph+=step_cph;
            fy+=step_fy;
            fph+=step_fph;
         }

         if(!port_map->sectors[port_map->walls[wall->wall].portal].visited)
            port_collect_walls(port_map->walls[wall->wall].portal);
      }
   }

   //printf("Max length: %d\n",max_len);

   //Render floor planes
   for(int i = 0;i<128;i++)
   {
      port_plane *pl = port_planes[i];
      while(pl!=NULL)
      {
         if(pl->min>pl->max)
         {
            pl = pl->next;
            continue;
         }

         //Parallax --> render differently
         if((port_map->sectors[pl->sector].flags&RVR_PORT_SECTOR_PARALLAX_FLOOR&&pl->where==1)||
            (port_map->sectors[pl->sector].flags&RVR_PORT_SECTOR_PARALLAX_CEILING&&pl->where==0))
         {
            RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;
            RvR_texture *texture = NULL;
            if(pl->where==1)
               texture = RvR_texture_get(port_map->sectors[pl->sector].floor_tex);
            else
               texture = RvR_texture_get(port_map->sectors[pl->sector].ceiling_tex);

            int skyw = 1 << RvR_log2(texture->width);
            int skyh = 1 << RvR_log2(texture->height);
            int mask = skyh - 1;

            RvR_fix16 angle_step = (skyw * 1024) / RvR_xres();
            RvR_fix16 tex_step = (1024 * skyh - 1) / RvR_yres();

            //TODO(Captain4LK): include fov in calculation
            RvR_fix22 angle = port_cam->dir*texture->width;
            angle += (pl->min - 1) * angle_step;

            for(int x = pl->min; x<pl->max + 1; x++)
            {
               //Sky is rendered fullbright, no lut needed
               uint8_t * restrict pix = &RvR_framebuffer()[(pl->start[x]) * RvR_xres() + x - 1];
               const uint8_t * restrict tex = &texture->data[((angle/1024) & (skyw - 1)) * skyh];
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
                  *pix = tex[texture_coord/1024];
                  texture_coord += tex_step;
                  pix += RvR_xres();

#if RVR_PORT_PIXELBYPIXEL
                  if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                     RvR_render_present();
#endif
               }
               RvR_fix16 tex_coord = (RvR_yres()) * tex_step - 1;
               texture_coord = RvR_min(tex_coord, tex_coord - tex_step * (tex_end - middle));
               for(int y = tex_end + 1; y<solid_end + 1; y++)
               {
                  *pix = col[tex[(texture_coord/1024) & mask]];
                  texture_coord -= tex_step;
                  pix += RvR_xres();

#if RVR_PORT_PIXELBYPIXEL
                  if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                     RvR_render_present();
#endif
               }

               angle += angle_step;
            }

            pl = pl->next;
            continue;
         }

         for(int x = pl->min;x<pl->max+2;x++)
         {
            RvR_fix22 s0 = pl->start[x-1];
            RvR_fix22 s1 = pl->start[x];
            RvR_fix22 e0 = pl->end[x-1];
            RvR_fix22 e1 = pl->end[x];

            //End spans top
            for(;s0<s1&&s0<=e0;s0++)
               port_span_draw(port_map,port_cam,pl->sector,pl->where,port_span_start[s0],x-1,s0);

            //End spans bottom
            for(;e0>e1&&e0>=s0;e0--)
               port_span_draw(port_map,port_cam,pl->sector,pl->where,port_span_start[e0],x-1,e0);

            //Start spans top
            for(;s1<s0&&s1<=e1;s1++)
               port_span_start[s1] = x-1;

            //Start spans bottom
            for(;e1>e0&&e1>=s1;e1--)
               port_span_start[e1] = x-1;
         }

         pl = pl->next;
      }
   
   }
}

void RvR_port_draw_end(RvR_port_selection *select)
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
      if(sp->flags & 8)
         port_sprite_draw_wall(sp, select);
      else if(sp->flags & 16)
         port_sprite_draw_floor(sp, select);
      else
         port_sprite_draw_billboard(port_map, sp, select);
   }
}

void RvR_port_draw_sprite(RvR_fix22 x, RvR_fix22 y, RvR_fix22 z, RvR_fix22 dir, uint16_t sector, uint16_t sprite, uint32_t flags, void *ref)
{
   //flagged as invisible
   if(flags & 1)
      return;

   //sector not visited
   if(!port_map->sectors[sector].visited)
      return;

   port_sprite sp = {0};
   sp.flags = flags;
   sp.texture = sprite;
   sp.x = x;
   sp.y = y;
   sp.z = z;
   sp.dir = dir;
   sp.ref = ref;

   RvR_texture *tex = RvR_texture_get(sprite);
   RvR_fix22 sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 cos = RvR_fix22_cos(port_cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres() * fovx, RvR_xres()*1024);
   RvR_fix22 sin_fov = RvR_fix22_mul(sin, fovx);
   RvR_fix22 cos_fov = RvR_fix22_mul(cos, fovx);
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;

   //Wall aligned
   if(flags & 8)
   {
      //Translate sprite to world space
      RvR_fix22 dirx = RvR_fix22_cos(dir);
      RvR_fix22 diry = RvR_fix22_sin(dir);
      RvR_fix22 half_width = (tex->width * 1024) / (64 * 2);
      RvR_fix22 p0x = RvR_fix22_mul(-diry, half_width) + x;
      RvR_fix22 p0y = RvR_fix22_mul(dirx, half_width) + y;
      RvR_fix22 p1x = RvR_fix22_mul(diry, half_width) + x;
      RvR_fix22 p1y = RvR_fix22_mul(-dirx, half_width) + y;
      sp.x = x;
      sp.y = y;
      sp.z = z;
      sp.as.wall.u0 = 0;
      sp.as.wall.u1 = 1024 * tex->width - 1;

      //Translate to camera space
      RvR_fix22 x0 = p0x - port_cam->x;
      RvR_fix22 y0 = p0y - port_cam->y;
      RvR_fix22 x1 = p1x - port_cam->x;
      RvR_fix22 y1 = p1y - port_cam->y;
      RvR_fix22 tp0x = RvR_fix22_mul(-x0, sin) + RvR_fix22_mul(y0, cos);
      RvR_fix22 tp0y = RvR_fix22_mul(x0, cos_fov) + RvR_fix22_mul(y0, sin_fov);
      RvR_fix22 tp1x = RvR_fix22_mul(-x1, sin) + RvR_fix22_mul(y1, cos);
      RvR_fix22 tp1y = RvR_fix22_mul(x1, cos_fov) + RvR_fix22_mul(y1, sin_fov);

      //Behind camera
      if(tp0y<-128&&tp1y<-128)
         return;

      //Sprite not facing camera
      //--> swap p0 and p1 and toggle y-axis mirror flag
      if(RvR_fix22_mul(tp0x, tp1y) - RvR_fix22_mul(tp1x, tp0y)>0)
      {
         //One sided sprite
         if(sp.flags & 128)
            return;

         RvR_fix22 tmp = tp0x;
         tp0x = tp1x;
         tp1x = tmp;

         tmp = tp0y;
         tp0y = tp1y;
         tp1y = tmp;
         sp.flags ^= 2;
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

         sp.as.wall.x0 = RvR_min(RvR_xres() * 512+ RvR_fix22_div(tp0x * (RvR_xres() / 2), tp0y), RvR_xres() * 1024);
         sp.as.wall.z0 = tp0y;
      }
      //Left point to the left of fov
      else
      {
         //Sprite completely out of sight
         if(tp1x<-tp1y)
            return;

         sp.as.wall.x0 = 0;
         RvR_fix22 dx0 = tp1x - tp0x;
         RvR_fix22 dx1 = tp0x + tp0y;
         sp.as.wall.z0 = RvR_fix22_div(RvR_fix22_mul(dx0, dx1), tp1y - tp0y + tp1x - tp0x) - tp0x;
         sp.as.wall.u0 = sp.as.wall.u0 + RvR_fix22_div(RvR_fix22_mul(-tp0x - tp0y, sp.as.wall.u1 - sp.as.wall.u0), RvR_non_zero(tp1x - tp0x + tp1y - tp0y));
      }

      //Right point in fov
      if(tp1x<=tp1y)
      {
         //sprite completely out of sight
         if(tp1x<-tp1y)
            return;

         sp.as.wall.x1 = RvR_min(RvR_xres() * 512+ RvR_fix22_div(tp1x * (RvR_xres() / 2), tp1y), RvR_xres() * 1024);
         sp.as.wall.z1 = tp1y;
      }
      //Right point to the right of fov
      else
      {
         //sprite completely out of sight
         if(tp0x>tp0y)
            return;

         RvR_fix22 dx0 = tp1x - tp0x;
         RvR_fix22 dx1 = tp0y - tp0x;
         sp.as.wall.x1 = RvR_xres() * 1024;
         sp.as.wall.z1 = tp0x - RvR_fix22_div(RvR_fix22_mul(dx0, dx1), tp1y - tp0y - tp1x + tp0x);
         sp.as.wall.u1 = RvR_fix22_div(RvR_fix22_mul(dx1, sp.as.wall.u1), RvR_non_zero(-tp1y + tp0y + tp1x - tp0x));
      }

      //Near clip sprite
      if(sp.as.wall.z0<128||sp.as.wall.z1<128)
         return;

      //Far clip sprite
      //TODO(Captain4LK): far clip once really far away (only a few/one pixels big)?
      //if(sp.as.wall.z0>RVR_RAY_MAX_STEPS * 65536&&sp.as.wall.z1>RVR_RAY_MAX_STEPS * 65536)
         //return;

      if(sp.as.wall.x0>sp.as.wall.x1)
         return;

      sp.z_min = RvR_min(sp.as.wall.z0, sp.as.wall.z1);
      sp.z_max = RvR_max(sp.as.wall.z0, sp.as.wall.z1);

      RvR_array_push(port_sprites, sp);

      return;
   }

   //Floor alligned
   if(flags & 16)
   {
      //World space coordinates, origin at camera
      RvR_fix22 scos = RvR_fix22_cos(dir);
      RvR_fix22 ssin = RvR_fix22_sin(dir);
      RvR_fix22 half_width = (tex->width * 1024) / (64 * 2);
      RvR_fix22 half_height = (tex->height * 1024) / (64 * 2);
      RvR_fix22 x0 = RvR_fix22_mul(-half_width, -ssin) + RvR_fix22_mul(-half_height, scos) + x - port_cam->x;
      RvR_fix22 y0 = RvR_fix22_mul(-half_width, scos) + RvR_fix22_mul(-half_height, ssin) + y - port_cam->y;
      RvR_fix22 x1 = RvR_fix22_mul(+half_width, -ssin) + RvR_fix22_mul(-half_height, scos) + x - port_cam->x;
      RvR_fix22 y1 = RvR_fix22_mul(+half_width, scos) + RvR_fix22_mul(-half_height, ssin) + y - port_cam->y;
      RvR_fix22 x2 = RvR_fix22_mul(+half_width, -ssin) + RvR_fix22_mul(+half_height, scos) + x - port_cam->x;
      RvR_fix22 y2 = RvR_fix22_mul(+half_width, scos) + RvR_fix22_mul(+half_height, ssin) + y - port_cam->y;
      RvR_fix22 x3 = RvR_fix22_mul(-half_width, -ssin) + RvR_fix22_mul(+half_height, scos) + x - port_cam->x;
      RvR_fix22 y3 = RvR_fix22_mul(-half_width, scos) + RvR_fix22_mul(+half_height, ssin) + y - port_cam->y;

      //Move to camera space
      sp.as.floor.x0 = RvR_fix22_mul(-x0, sin) + RvR_fix22_mul(y0, cos);
      sp.as.floor.z0 = RvR_fix22_mul(x0, cos_fov) + RvR_fix22_mul(y0, sin_fov);
      sp.as.floor.x1 = RvR_fix22_mul(-x1, sin) + RvR_fix22_mul(y1, cos);
      sp.as.floor.z1 = RvR_fix22_mul(x1, cos_fov) + RvR_fix22_mul(y1, sin_fov);
      sp.as.floor.x2 = RvR_fix22_mul(-x2, sin) + RvR_fix22_mul(y2, cos);
      sp.as.floor.z2 = RvR_fix22_mul(x2, cos_fov) + RvR_fix22_mul(y2, sin_fov);
      sp.as.floor.x3 = RvR_fix22_mul(-x3, sin) + RvR_fix22_mul(y3, cos);
      sp.as.floor.z3 = RvR_fix22_mul(x3, cos_fov) + RvR_fix22_mul(y3, sin_fov);

      sp.as.floor.wx = RvR_fix22_mul(-(x - port_cam->x), sin) + RvR_fix22_mul(y - port_cam->y, cos);
      sp.as.floor.wy = RvR_fix22_mul(x - port_cam->x, cos_fov) + RvR_fix22_mul(y - port_cam->y, sin_fov);

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
   sp.as.bill.wx = RvR_fix22_mul(-(x - port_cam->x), sin) + RvR_fix22_mul(y - port_cam->y, cos);
   sp.as.bill.wy = RvR_fix22_mul(x - port_cam->x, cos_fov) + RvR_fix22_mul(y - port_cam->y, sin_fov);
   sp.z_min = sp.z_max = sp.as.bill.wy;
   RvR_fix22 depth = RvR_fix22_mul(x - port_cam->x, cos) + RvR_fix22_mul(y - port_cam->y, sin); //Separate depth, so that the near/far clip is not dependent on fov

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

   //Above screen
   if(middle_row * RvR_fix22_mul(depth, fovy)<(RvR_yres()/2) * (z - port_cam->z))
      return;

   //Below screen
   if((middle_row - RvR_yres()) * RvR_fix22_mul(depth, fovy)>(RvR_yres()/2) * (z - port_cam->z + tex->height * 16))
      return;

   RvR_array_push(port_sprites, sp);
}

const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_floor(int x)
{
   return port_depth_buffer.floor[x];
}

const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_ceiling(int x)
{
   return port_depth_buffer.ceiling[x];
}

//Calculates wether wa can be drawn in front of wb
static int dwall_can_front(const RvR_port_dwall *wa, const RvR_port_dwall *wb)
{
   int64_t x00 = wa->xw0;
   int64_t z00 = wa->zw0;
   int64_t x01 = wa->xw1;
   int64_t z01 = wa->zw1;
   int64_t x10 = wb->xw0;
   int64_t z10 = wb->zw0;
   int64_t x11 = wb->xw1;
   int64_t z11 = wb->zw1;

   int64_t dx0 = x01-x00;
   int64_t dz0 = z01-z00;
   int64_t dx1 = x11-x10;
   int64_t dz1 = z11-z10;

   int64_t cross00 = dx0*(z10-z00)-dz0*(x10-x00);
   int64_t cross01 = dx0*(z11-z00)-dz0*(x11-x00);
   int64_t cross10 = dx1*(z00-z10)-dz1*(x00-x10);
   int64_t cross11 = dx1*(z01-z10)-dz1*(x01-x10);

   //wb completely behind wa
   if(RvR_min(z10,z11)>RvR_max(z00,z01))
      return 1;

   //no overlap on screen
   if(wa->x0>=wb->x1||wa->x1<=wb->x0)
      return 1;

   //p0 and p1 of wb behind wa
   if(cross00>=0&&cross01>=0)
      return 1;

   //p0 and p1 of wa in front of wb
   if(cross10<=0&&cross11<=0)
      return 1;
   
   //Need swapping
   return 0;
}

static void port_span_draw(const RvR_port_map *map, const RvR_port_cam *cam, int16_t sector, uint8_t where, int x0, int x1, int y)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   //TODO: investigate this further
   //Happens when a floor plane gets drawn at the horizon (should this happen?)
   if(y==RvR_yres()/2)
      return;

   RvR_texture *texture = NULL;
   if(where==0)
      texture = RvR_texture_get(map->sectors[sector].ceiling_tex);
   else if(where==1)
      texture = RvR_texture_get(map->sectors[sector].floor_tex);

   if(texture==NULL)
      return;

   int8_t shade_off = 0;
   RvR_fix22 height = 0;
   if(where==0)
   {
      height = map->sectors[sector].ceiling;
      shade_off = map->sectors[sector].shade_ceiling;
   }
   else if(where==1)
   {
      height = map->sectors[sector].floor;
      shade_off = map->sectors[sector].shade_floor;
   }

   RvR_fix22 view_sin = RvR_fix22_sin(cam->dir);
   RvR_fix22 view_cos = RvR_fix22_cos(cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(cam->fov/2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres()*fovx,RvR_xres()*1024);

   RvR_fix22 span_height = (height-cam->z);
   RvR_fix22 fdy = RvR_abs(RvR_yres()/2-y)*1024;

   int64_t slope = RvR_abs((span_height*(INT64_C(1)<<30))/RvR_non_zero(fdy));
   RvR_fix22 depth = RvR_fix22_div((RvR_yres()/2)*span_height,RvR_non_zero(RvR_fix22_mul(fovy,fdy)));
   depth = RvR_abs(depth);

   RvR_fix22 step_x = (RvR_fix22)(((int64_t)view_sin*slope)/(1<<18));
   RvR_fix22 step_y = (RvR_fix22)(((int64_t)view_cos*slope)/(1<<18));
   RvR_fix22 tx,ty;

   if((where==1&&map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_FLOOR)||
      (where==0&&map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_CEILING))
   {
      RvR_fix22 wx0 = map->walls[map->sectors[sector].wall_first].x;
      RvR_fix22 wy0 = map->walls[map->sectors[sector].wall_first].y;
      tx = -(cam->x-wx0)*1024*4-4*view_cos*depth+(x0-RvR_xres()/2)*step_x;;
      ty = (cam->y-wy0)*1024*4+4*view_sin*depth+(x0-RvR_xres()/2)*step_y;;
   }
   else
   {
      tx = -(cam->x&4095)*1024*4-4*view_cos*depth+(x0-RvR_xres()/2)*step_x;;
      ty = (cam->y&4095)*1024*4+4*view_sin*depth+(x0-RvR_xres()/2)*step_y;;
   }

   //Offset textures
   tx+=((int32_t)map->sectors[sector].x_off)*65536;
   ty+=((int32_t)map->sectors[sector].y_off)*65536;

   //Rotate textures
   if((where==1&&map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_FLOOR)||
      (where==0&&map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_CEILING))
   {
      RvR_fix22 wx0 = map->walls[map->sectors[sector].wall_first].x;
      RvR_fix22 wy0 = map->walls[map->sectors[sector].wall_first].y;
      RvR_fix22 wx1 = map->walls[map->walls[map->sectors[sector].wall_first].p2].x;
      RvR_fix22 wy1 = map->walls[map->walls[map->sectors[sector].wall_first].p2].y;
      RvR_fix22 len = RvR_non_zero(RvR_fix22_sqrt(RvR_fix22_mul(wx0-wx1,wx0-wx1)+RvR_fix22_mul(wy0-wy1,wy0-wy1)));
      RvR_fix22 sp_cos = RvR_fix22_div(wx1-wx0,len);
      RvR_fix22 sp_sin = -RvR_fix22_div(wy1-wy0,len);
      RvR_fix22 tmp = tx;
      tx = RvR_fix22_mul(-sp_sin, tx) + RvR_fix22_mul(sp_cos, ty);
      ty = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, ty);
      tmp = step_x;
      step_x = RvR_fix22_mul(-sp_sin, step_x) + RvR_fix22_mul(sp_cos, step_y);
      step_y = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, step_y);
   }

   //Rotate 90 degrees
   if((where==1&&map->sectors[sector].flags&RVR_PORT_SECTOR_ROT_FLOOR)||
      (where==0&&map->sectors[sector].flags&RVR_PORT_SECTOR_ROT_CEILING))
   {
   }

   RvR_fix22 x_log = RvR_log2(texture->width);
   RvR_fix22 y_log = RvR_log2(texture->height);
   RvR_fix22 x_and = (1<<x_log)-1;
   RvR_fix22 y_and = (1<<y_log)-1;
   y_log = RvR_max(0,16-y_log);
   x_and<<=(16-y_log);

   uint8_t * restrict pix = RvR_framebuffer()+y*RvR_xres()+x0;
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(depth>>9)+shade_off)));
   const uint8_t * restrict tex = texture->data;

   for(int x = x0;x<x1;x++)
   {
      uint8_t c = tex[((tx>>y_log)&x_and)+((ty>>16)&y_and)];
      *pix = col[c];
      tx+=step_x;
      ty+=step_y;
      pix++;

#if RVR_PORT_PIXELBYPIXEL
      if(RvR_key_pressed(RVR_PORT_PIXELKEY))
         RvR_render_present();
#endif
   }
}

static void port_plane_add(int16_t sector, uint8_t where, int x, int y0, int y1)
{
   x+=1;

   //TODO: check how this hash actually performs (probably poorly...)
   int hash = (sector*7+where*3)&127;

   port_plane *pl = port_planes[hash];
   while(pl!=NULL)
   {
      //port_planes need to be in the same sector..
      if(sector!=pl->sector)
         goto next;
      //... and the same texture to be valid for concatination
      if(where!=pl->where)
         goto next;

      //Additionally the spans collumn needs to be either empty...
      if(pl->start[x]!=UINT16_MAX)
      {
         //... or directly adjacent to each other vertically, in that case
         //Concat planes vertically
         if(pl->start[x]-1==y1)
         {
            pl->start[x] = (uint16_t)y0;
            return;
         }
         if(pl->end[x]+1==y0)
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
      pl = port_plane_new();
      pl->next= port_planes[hash];
      port_planes[hash] = pl;

      pl->min = RvR_xres();
      pl->max = -1;
      pl->sector = sector;
      pl->where = where;

      //Since this is an unsigned int, we can use memset to set all values to 65535 (0xffff)
      memset(pl->start,255,sizeof(pl->start));
   }

   if(x<pl->min)
      pl->min = x;
   if(x>pl->max)
      pl->max = x;

   pl->end[x] = (uint16_t)y1;
   pl->start[x] = (uint16_t)y0;
}

static port_plane *port_plane_new()
{
   if(port_plane_pool==NULL)
   {
      port_plane *p = RvR_malloc(sizeof(*p)*8,"RvR_port plane pool");
      memset(p,0,sizeof(*p)*8);

      for(int i = 0;i<7;i++)
         p[i].next = &p[i+1];
      port_plane_pool = p;
   }

   port_plane *p = port_plane_pool;
   port_plane_pool = p->next;
   p->next = NULL;

   return p;
}

static void port_plane_free(port_plane *pl)
{
   if(pl==NULL)
      return;

   //Find last
   port_plane *last = pl;
   while(last->next!=NULL)
      last = last->next;

   last->next = port_plane_pool;
   port_plane_pool = pl;
}

static RvR_port_depth_buffer_entry *port_depth_buffer_entry_new()
{
   if(port_depth_buffer_entry_pool==NULL)
   {
      RvR_port_depth_buffer_entry *e = RvR_malloc(sizeof(*e) * 256, "RvR_port port_depth_buffer_entry pool");
      memset(e, 0, sizeof(*e) * 256);

      for(int i = 0; i<255; i++)
         e[i].next = &e[i + 1];
      port_depth_buffer_entry_pool = e;
   }

   RvR_port_depth_buffer_entry *e = port_depth_buffer_entry_pool;
   port_depth_buffer_entry_pool = e->next;
   e->next = NULL;

   return e;
}

static void port_depth_buffer_entry_free(RvR_port_depth_buffer_entry *ent)
{
   if(ent==NULL)
      return;

   //Find last
   RvR_port_depth_buffer_entry *last = ent;
   while(last->next!=NULL)
      last = last->next;

   last->next = port_depth_buffer_entry_pool;
   port_depth_buffer_entry_pool = ent;
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
   if(a->flags & 8&&b->flags & 8)
      return port_wsprite_can_back(a, b);

   //Sprite - Sprite check
   //is a lot simpler
   if(!(a->flags & 8)&&!(b->flags & 8))
   {
      //If one is floor sprite, check height
      if(a->flags & 16||b->flags & 16)
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
   if(a->flags & 8)
   {
      x00 = a->as.wall.wx0;
      x01 = a->as.wall.wx1;
      z00 = a->as.wall.wy0;
      z01 = a->as.wall.wy1;
   }
   else if(a->flags & 16)
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

   if(b->flags & 8)
   {
      x10 = b->as.wall.wx0;
      x11 = b->as.wall.wx1;
      z10 = b->as.wall.wy0;
      z11 = b->as.wall.wy1;
   }
   else if(b->flags & 16)
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
   if(a->flags & 8)
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

   int64_t dx0 = x01 - x00;
   int64_t dz0 = z01 - z00;
   int64_t dx1 = x11 - x10;
   int64_t dz1 = z11 - z10;

   //2d cross product/perp dot product
   int64_t cross00 = dx0 * (z10 - z00) - dz0 * (x10 - x00);
   int64_t cross01 = dx0 * (z11 - z00) - dz0 * (x11 - x00);
   int64_t cross10 = dx1 * (z00 - z10) - dz1 * (x00 - x10);
   int64_t cross11 = dx1 * (z01 - z10) - dz1 * (x01 - x10);

   //All points of b in front of a
   if(cross00<=0&&cross01<=0)
      return 1;

   //All points of a behind b
   if(cross10>=0&&cross11>=0)
      return 1;

   //Need swapping
   return 0;
}

static void port_sprite_draw_billboard(const RvR_port_map *map, const port_sprite *sp, RvR_port_selection *select)
{
   RvR_fix22 cos = RvR_fix22_cos(port_cam->dir);
   RvR_fix22 sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres() * fovx, RvR_xres()*1024);
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;

   RvR_texture *texture = RvR_texture_get(sp->texture);

   RvR_fix22 tpx = sp->x - port_cam->x;
   RvR_fix22 tpy = sp->y - port_cam->y;
   RvR_fix22 depth = RvR_fix22_mul(tpx, cos) + RvR_fix22_mul(tpy, sin);
   tpx = RvR_fix22_mul(-tpx, sin) + RvR_fix22_mul(tpy, cos);

   //Dimensions
   RvR_fix22 top = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z + texture->height * 16), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int y0 = (top + 1023) / 1024;

   RvR_fix22 bot = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int y1 = (bot - 1) / 1024;

   RvR_fix22 left = RvR_xres() * 512+ RvR_fix22_div((RvR_xres() / 2) * (tpx - texture->width * 8), RvR_non_zero(RvR_fix22_mul(depth, fovx)));
   int x0 = (left + 1023) / 1024;

   RvR_fix22 right = RvR_xres() * 512+ RvR_fix22_div((RvR_xres() / 2) * (tpx + texture->width * 8), RvR_non_zero(RvR_fix22_mul(depth, fovx)));
   int x1 = (right - 1) / 1024;

   //Floor and ceiling clip
   RvR_fix22 cy = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (map->sectors[sp->sector].floor - port_cam->z), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int clip_bottom = RvR_min(cy / 1024, RvR_yres());
   y1 = RvR_min(y1, clip_bottom);

   cy = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (map->sectors[sp->sector].ceiling - port_cam->z), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int clip_top = RvR_max(cy / 1024, 0);
   y0 = RvR_max(y0, clip_top);

   x1 = RvR_min(x1, RvR_xres());
   RvR_fix22 step_v = (8*fovy* depth) / RvR_yres();
   RvR_fix22 step_u = (8 * fovx* depth) / RvR_xres();
   RvR_fix22 u = RvR_fix22_mul(step_u, x0 * 1024- left);

   if(x0<0)
   {
      u += (-x0) * step_u;
      x0 = 0;
      left = 0;
   }

   //Vertical flip
   if(sp->flags & 4)
      step_v = -step_v;

   //Draw
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_min(63, depth >> 15));
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;
   for(int x = x0; x<x1; x++)
   {
      //Clip against walls
      int ys = y0;
      int ye = y1;

      //Clip floor
      RvR_port_depth_buffer_entry *clip = port_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ye>clip->limit)
            ye = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      clip = port_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
            ys = clip->limit;
         clip = clip->next;
      }

      int tu = u / 65536;
      if(sp->flags & 2)
         tu = texture->width - tu - 1;
      tex = &texture->data[texture->height * (tu)];
      dst = &RvR_framebuffer()[ys * RvR_xres() + x];

      RvR_fix22 v = (sp->z - port_cam->z)*4096 + (ys - middle_row + 1) * step_v + texture->height * 65536;
      if(sp->flags & 4)
         v = texture->height * 1024 - ((sp->z - port_cam->z) + (ys - middle_row + 1) * (-step_v) + texture->height * 1024);

      if(select!=NULL&&select->x==x&&select->y>=ys&&select->y<ye&&select->depth>depth)
      {
         //Check for transparent pixels
         if(tex[(v + step_v * (select->y - ys)) >> 16])
         {
            //TODO(Captain4LK): sprite pointer
            select->depth = depth;
            select->type = RVR_PORT_SPRITE_BILL;
         }
      }

      if(sp->flags & 32)
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 16)];
            *dst = RvR_blend(col[index], *dst);
            v += step_v;
         }
      }
      else if(sp->flags & 64)
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 16)];
            *dst = RvR_blend(*dst, col[index]);
            v += step_v;
         }
      }
      else
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 16)];
            *dst = index?col[index]:*dst;
            v += step_v;
         }
      }

      u += step_u;
   }
}

static void port_sprite_draw_wall(const port_sprite *sp, RvR_port_selection *select)
{
   RvR_texture *texture = RvR_texture_get(sp->texture);
   RvR_fix22 scale_vertical = texture->height * 16; //texture height in map coordinates
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres() * fovx, RvR_xres()*1024);
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;

   RvR_fix22 cy0 = RvR_fix22_div((RvR_yres()/2) * (sp->z + scale_vertical - port_cam->z), RvR_fix22_mul(sp->as.wall.z0, fovy));
   RvR_fix22 cy1 = RvR_fix22_div((RvR_yres()/2) * (sp->z + scale_vertical - port_cam->z), RvR_fix22_mul(sp->as.wall.z1, fovy));
   cy0 = middle_row * 1024- cy0;
   cy1 = middle_row * 1024- cy1;
   RvR_fix22 step_cy = RvR_fix22_div(cy1 - cy0, RvR_non_zero(sp->as.wall.x1 - sp->as.wall.x0));
   RvR_fix22 cy = cy0;

   RvR_fix22 fy0 = RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z), RvR_fix22_mul(sp->as.wall.z0, fovy));
   RvR_fix22 fy1 = RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z), RvR_fix22_mul(sp->as.wall.z1, fovy));
   fy0 = middle_row * 1024- fy0;
   fy1 = middle_row * 1024- fy1;
   RvR_fix22 step_fy = RvR_fix22_div(fy1 - fy0, RvR_non_zero(sp->as.wall.x1 - sp->as.wall.x0));
   RvR_fix22 fy = fy0;

   //1/z and u/z can be interpolated linearly
   //Instead of actually calculating 1/z (which would be imprecise for fixed point)
   //we bring both 1/z and u/z on the common denominator (z0*z1*w) and interpolate
   //the numerators instead
   RvR_fix22 denom = RvR_fix22_mul(sp->as.wall.z1,sp->as.wall.z0);
   RvR_fix22 num_step_z = 4*(sp->as.wall.z0-sp->as.wall.z1);
   RvR_fix22 num_z = 4*sp->as.wall.z1;

   RvR_fix22 num_step_u = RvR_fix22_mul(sp->as.wall.z0,sp->as.wall.u1)-RvR_fix22_mul(sp->as.wall.z1,sp->as.wall.u0);
   RvR_fix22 num_u = RvR_fix22_mul(sp->as.wall.u0,sp->as.wall.z1);

   //Adjust for fractional part
   int x0 = (sp->as.wall.x0 + 1023)/1024;
   int x1 = (sp->as.wall.x1 + 1023)/1024;
   RvR_fix22 xfrac = sp->as.wall.x0 - x0 * 1024;
   cy -= RvR_fix22_mul(xfrac, step_cy);
   fy -= RvR_fix22_mul(xfrac, step_fy);
   num_z-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
   num_u-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_u),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));

   for(int x = x0; x<x1; x++)
   {
      int y0 = (cy + 1023) / 1024;
      int y1 = fy / 1024;

      RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
      RvR_fix22 nu = num_u+RvR_fix22_div(num_step_u*(x-x0),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
      RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz/4));
      RvR_fix22 u = (4*nu)/RvR_non_zero(nz);

      //Clip floor
      int ybot = RvR_yres() - 1;
      RvR_port_depth_buffer_entry *clip = port_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ybot>clip->limit)
            ybot = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      int ytop = 0;
      clip = port_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ytop<clip->limit)
            ytop = clip->limit;
         clip = clip->next;
      }

      int wy =  ytop;
      uint8_t * restrict pix = RvR_framebuffer() + (wy * RvR_xres() + x);

      int y_to = RvR_min(y0, ybot);
      if(y_to>wy)
      {
         wy = y_to;
         pix = RvR_framebuffer() + (wy * RvR_xres() + x);
      }

      //Wall
      //Inverting the texture coordinates directly didn't work properly,
      //so we just invert u here.
      //TODO: investigate this
      if(sp->flags & 2)
         u = texture->width - u - 1;
      RvR_fix22 height = sp->z + scale_vertical - port_cam->z;
      RvR_fix22 coord_step_scaled = (8*fovy*depth)/RvR_yres();
      RvR_fix22 texture_coord_scaled = height*4096+(wy-RvR_yres()/2+1)*coord_step_scaled;
      //Vertical flip
      if(sp->flags & 4)
      {
         coord_step_scaled = -coord_step_scaled;
         texture_coord_scaled = texture->height * 16- height + (wy - middle_row + 1) * coord_step_scaled;
      }
      const uint8_t * restrict tex = &texture->data[(((uint32_t)u) % texture->width) * texture->height];
      const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0, RvR_min(63, (depth >> 15))));


      y_to = RvR_min(y1, ybot);

      if(select!=NULL&&select->x==x&&select->y>=wy&&select->y<y_to&&select->depth>depth)
      {
         //Check for transparent pixels
         if(tex[(texture_coord_scaled + coord_step_scaled * (select->y - wy)) >> 16])
         {
            //TODO(Captain4LK): sprite pointer
            select->depth = depth;
            select->type = RVR_PORT_SPRITE_WALL;
         }
      }

      if(sp->flags & 32)
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 16)];
            *pix = RvR_blend(col[index], *pix);
            pix += RvR_xres();
            texture_coord_scaled += coord_step_scaled;
         }
      }
      else if(sp->flags & 64)
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 16)];
            *pix = RvR_blend(*pix, col[index]);
            pix += RvR_xres();
            texture_coord_scaled += coord_step_scaled;
         }
      }
      else
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 16)];
            *pix = index?col[index]:*pix;
            pix += RvR_xres();
            texture_coord_scaled += coord_step_scaled;
         }
      }

      cy += step_cy;
      fy += step_fy;
   }
}

static void port_sprite_draw_floor(const port_sprite *sp, RvR_port_selection *select)
{
   //After clipping we will never have more than 8 vertices
   RvR_fix22 verts[8][3];
   RvR_fix22 verts2[8][3];
   int verts_count = 4;
   int verts2_count = 0;
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres() * fovx, RvR_xres()*1024);
   RvR_texture *texture = RvR_texture_get(sp->texture);
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;
   RvR_fix22 middle_row2 = (RvR_yres() / 2) - port_cam->shear;

   verts[0][0] = sp->as.floor.x0;
   verts[0][1] = sp->as.floor.z0;
   verts[0][2] = (sp->z - port_cam->z) * (RvR_yres()/2);
   verts[1][0] = sp->as.floor.x1;
   verts[1][1] = sp->as.floor.z1;
   verts[1][2] = (sp->z - port_cam->z) * (RvR_yres()/2);
   verts[2][0] = sp->as.floor.x2;
   verts[2][1] = sp->as.floor.z2;
   verts[2][2] = (sp->z - port_cam->z) * (RvR_yres()/2);
   verts[3][0] = sp->as.floor.x3;
   verts[3][1] = sp->as.floor.z3;
   verts[3][2] = (sp->z - port_cam->z) * (RvR_yres()/2);

   //Clip to view
   //-------------------------------
   //Clip left
   verts2_count = 0;
   RvR_fix22 left = verts[0][0] + verts[0][1];
   for(int i = 0; i<verts_count; i++)
   {
      int p2 = (i + 1) % verts_count;
      RvR_fix22 leftn = verts[p2][0] + verts[p2][1];
      if(left>=0)
      {
         verts2[verts2_count][0] = verts[i][0];
         verts2[verts2_count][1] = verts[i][1];
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }
      if((left ^ leftn)<0)
      {
         verts2[verts2_count][0] = verts[i][0] + RvR_fix22_mul(RvR_fix22_div(left, left - leftn), verts[p2][0] - verts[i][0]);
         verts2[verts2_count][1] = verts[i][1] + RvR_fix22_mul(RvR_fix22_div(left, left - leftn), verts[p2][1] - verts[i][1]);
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }

      left = leftn;
   }
   if(verts2_count<=2)
      return;

   //Clip right
   verts_count = 0;
   RvR_fix22 right = verts2[0][0] - verts2[0][1];
   for(int i = 0; i<verts2_count; i++)
   {
      int p2 = (i + 1) % verts2_count;
      RvR_fix22 rightn = verts2[p2][0] - verts2[p2][1];
      if(right<=0)
      {
         verts[verts_count][0] = verts2[i][0];
         verts[verts_count][1] = verts2[i][1];
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }
      if((right ^ rightn)<0)
      {
         verts[verts_count][0] = verts2[i][0] + RvR_fix22_mul(RvR_fix22_div(right, right - rightn), verts2[p2][0] - verts2[i][0]);
         verts[verts_count][1] = verts2[i][1] + RvR_fix22_mul(RvR_fix22_div(right, right - rightn), verts2[p2][1] - verts2[i][1]);
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }

      right = rightn;
   }
   if(verts_count<=2)
      return;

   //Clip bottom
   verts2_count = 0;
   RvR_fix22 down = verts[0][2] + RvR_fix22_mul(verts[0][1], fovy) * middle_row2;
   for(int i = 0; i<verts_count; i++)
   {
      int p2 = (i + 1) % verts_count;
      RvR_fix22 downn = verts[p2][2] + RvR_fix22_mul(verts[p2][1], fovy) * middle_row2;

      if(down>=0)
      {
         verts2[verts2_count][0] = verts[i][0];
         verts2[verts2_count][1] = verts[i][1];
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }
      if((down ^ downn)<0)
      {
         verts2[verts2_count][0] = verts[i][0] + RvR_fix22_div(RvR_fix22_mul(down, verts[p2][0] - verts[i][0]), down - downn);
         verts2[verts2_count][1] = verts[i][1] + RvR_fix22_div(RvR_fix22_mul(down, verts[p2][1] - verts[i][1]), down - downn);
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }

      down = downn;
   }
   if(verts2_count<=2)
      return;

   //Clip top
   verts_count = 0;
   RvR_fix22 up = verts2[0][2] + RvR_fix22_mul(verts2[0][1], fovy) * (middle_row2 - RvR_yres());
   for(int i = 0; i<verts2_count; i++)
   {
      int p2 = (i + 1) % verts2_count;
      RvR_fix22 upn = verts2[p2][2] + RvR_fix22_mul(verts2[p2][1], fovy) * (middle_row2 - RvR_yres());

      if(up<=0)
      {
         verts[verts_count][0] = verts2[i][0];
         verts[verts_count][1] = verts2[i][1];
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }
      if((up ^ upn)<0)
      {
         verts[verts_count][0] = verts2[i][0] + RvR_fix22_mul(RvR_fix22_div(up, RvR_non_zero(up - upn)), verts2[p2][0] - verts2[i][0]);
         verts[verts_count][1] = verts2[i][1] + RvR_fix22_mul(RvR_fix22_div(up, RvR_non_zero(up - upn)), verts2[p2][1] - verts2[i][1]);
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }

      up = upn;
   }
   if(verts_count<=2)
      return;

   //Project to screen
   for(int i = 0; i<verts_count; i++)
   {
      verts[i][0] = RvR_max(0, RvR_min(RvR_xres() * 1024- 1, RvR_xres() * 512+ RvR_fix22_div(verts[i][0] * (RvR_xres() / 2), RvR_non_zero(verts[i][1]))));
      verts[i][2] = RvR_max(0, RvR_min(RvR_yres() * 1024- 1, middle_row * 1024- RvR_fix22_div(verts[i][2], RvR_non_zero(RvR_fix22_mul(verts[i][1], fovy)))));
   }
   //-------------------------------

   //Rasterize
   //-------------------------------
   int index_minl = 0;
   RvR_fix22 xmin = RvR_xres() * 1024;
   for(int i = 0; i<verts_count; i++)
   {
      if(verts[i][0]<xmin)
      {
         xmin = verts[i][0];
         index_minl = i;
      }
   }

   int index_minr = index_minl;

   RvR_fix22 le_y = 0;
   RvR_fix22 le_dy = 0;
   RvR_fix22 le_width = 0;
   RvR_fix22 re_y = 0;
   RvR_fix22 re_dy = 0;
   RvR_fix22 re_width = 0;

   int x = (xmin + 1023) / 1024;

   int prev_start = RvR_yres();
   int prev_end = 0;
   for(;;)
   {
      if(!le_width)
      {
         int index_rightl = (index_minl - 1 + verts_count) % verts_count;
         le_width = (verts[index_rightl][0] + 1023) / 1024- (verts[index_minl][0] + 1023) / 1024;
         if(le_width<0)
            break;

         le_dy = RvR_fix22_div(verts[index_rightl][2] - verts[index_minl][2], RvR_non_zero(verts[index_rightl][0] - verts[index_minl][0]));
         le_y = verts[index_minl][2] + RvR_fix22_mul(le_dy, ((verts[index_minl][0] + 1023) / 1024) * 1024- verts[index_minl][0]);

         index_minl = index_rightl;
      }

      if(!re_width)
      {
         int index_rightr = (index_minr + 1) % verts_count;
         re_width = (verts[index_rightr][0] + 1023) / 1024- (verts[index_minr][0] + 1023) / 1024;
         if(re_width<0)
            break;

         re_dy = RvR_fix22_div(verts[index_rightr][2] - verts[index_minr][2], RvR_non_zero(verts[index_rightr][0] - verts[index_minr][0]));
         re_y = verts[index_minr][2] + RvR_fix22_mul(re_dy, ((verts[index_minr][0] + 1023) / 1024) * 1024- verts[index_minr][0]);

         index_minr = index_rightr;
      }

      if(!re_width&&!le_width)
         break;

      int width = RvR_min(le_width, re_width);

      le_width -= width;
      re_width -= width;

      while(width-->0)
      {
         int start = RvR_max(0, RvR_min(RvR_yres() - 1, (le_y + 1023) / 1024));
         int end = RvR_max(0, RvR_min(RvR_yres() - 1, (re_y + 1023) / 1024));
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
         RvR_port_depth_buffer_entry *clip = port_depth_buffer.floor[x];
         while(clip!=NULL)
         {
            if(sp->as.floor.wy>clip->depth&&end>clip->limit)
               end = clip->limit;
            clip = clip->next;
         }

         //Clip ceiling
         clip = port_depth_buffer.ceiling[x];
         while(clip!=NULL)
         {
            if(sp->as.floor.wy>clip->depth&&start<clip->limit)
               start = clip->limit;
            clip = clip->next;
         }

         if(select!=NULL)
         {
            if(x==select->x&&select->y>=start&&select->y<=end&&select->depth>sp->as.floor.wy)
            {
               //TODO(Captian4LK): sprite pointer
               select->depth = sp->as.floor.wy;
               select->type = RVR_PORT_SPRITE_FLOOR;
            }
         }

         RvR_fix22 s0 = prev_start;
         RvR_fix22 s1 = start;
         RvR_fix22 e0 = prev_end;
         RvR_fix22 e1 = end;

         //End spans top
         for(; s0<s1&&s0<=e0; s0++)
            port_floor_span_draw(sp, port_span_start[s0], x, s0, texture);

         //End spans bottom
         for(; e0>e1&&e0>=s0; e0--)
            port_floor_span_draw(sp, port_span_start[e0], x, e0, texture);

         //Start spans top
         for(; s1<s0&&s1<=e1; s1++)
            port_span_start[s1] = x;

         //Start spans bottom
         for(; e1>e0&&e1>=s1; e1--)
            port_span_start[e1] = x;

         prev_start = start;
         prev_end = end;
         le_y += le_dy;
         re_y += re_dy;
         x++;
      }
   }

   //End remaining
   RvR_fix22 s0 = prev_start;
   RvR_fix22 s1 = prev_end;
   for(; s0<=s1; s0++)
      port_floor_span_draw(sp, port_span_start[s0], x, s0, texture);
   //-------------------------------
}

static void port_floor_span_draw(const port_sprite *sp, int x0, int x1, int y, const RvR_texture *texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   if(texture==NULL)
      return;

   RvR_fix22 view_sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 view_cos = RvR_fix22_cos(port_cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres() * fovx, RvR_xres()*1024);
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;

   RvR_fix22 dy = middle_row - y;
   RvR_fix22 depth = RvR_fix22_div(RvR_abs(port_cam->z - sp->z), RvR_non_zero(fovy));
   depth = RvR_fix22_div(depth * (RvR_yres()/2), RvR_non_zero(RvR_abs(dy)*1024)); //TODO

   RvR_fix22 x_log = RvR_log2(texture->width);
   RvR_fix22 y_log = RvR_log2(texture->height);
   RvR_fix22 step_x = ((view_sin*(port_cam->z-sp->z))/RvR_non_zero(dy));
   RvR_fix22 step_y = ((view_cos*(port_cam->z-sp->z))/RvR_non_zero(dy));
   RvR_fix22 tx = (port_cam->x-sp->x)*1024+(view_cos*depth)+(x0-RvR_xres()/2)*step_x;
   RvR_fix22 ty = -(port_cam->y-sp->y)*1024-(view_sin*depth)+(x0-RvR_xres()/2)*step_y;
   RvR_fix22 x_and = (1 << x_log) - 1;
   RvR_fix22 y_and = (1 << y_log) - 1;

   //Rotate texture coordinates according to sprite rotation
   RvR_fix22 sp_sin = RvR_fix22_sin(-sp->dir);
   RvR_fix22 sp_cos = RvR_fix22_cos(-sp->dir);
   RvR_fix22 tmp = tx;
   tx = RvR_fix22_mul(-sp_sin, tx) + RvR_fix22_mul(sp_cos, ty);
   ty = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, ty);
   tmp = step_x;
   step_x = RvR_fix22_mul(-sp_sin, step_x) + RvR_fix22_mul(sp_cos, step_y);
   step_y = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, step_y);

   //Offset texture coordinates
   //since sprites are anchored in their middle
   tx += texture->width * 8192;
   ty += texture->height * 8192;

   uint8_t * restrict pix = RvR_framebuffer() + y * RvR_xres() + x0;
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0, RvR_min(63, (depth >> 15))));
   const uint8_t * restrict tex = texture->data;

   if(sp->flags & 32)
   {
      for(int x = x0; x<x1; x++)
      {
         uint8_t c = tex[(((tx >> 14) & x_and) << y_log) + ((ty >> 14) & y_and)];
         *pix = RvR_blend(col[c], *pix);
         tx += step_x;
         ty += step_y;
         pix++;
      }
   }
   else if(sp->flags & 64)
   {
      for(int x = x0; x<x1; x++)
      {
         uint8_t c = tex[(((tx >> 14) & x_and) << y_log) + ((ty >> 14) & y_and)];
         *pix = RvR_blend(*pix, col[c]);
         tx += step_x;
         ty += step_y;
         pix++;
      }
   }
   else
   {
      for(int x = x0; x<x1; x++)
      {
         uint8_t c = tex[(((tx >> 14) & x_and) << y_log) + ((ty >> 14) & y_and)];
         *pix = c?col[c]:*pix;
         tx += step_x;
         ty += step_y;
         pix++;
      }
   }
}

static void port_collect_walls(int16_t start)
{
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov/2);
   RvR_fix22 sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 cos = RvR_fix22_cos(port_cam->dir);
   //Instead of correcting for fov later, we premultiply
   //here, this allows us to assume that the edges
   //of the view frustumg are lines with a slope of
   //one through the origin
   RvR_fix22 sin_fov = RvR_fix22_mul(sin,fovx);
   RvR_fix22 cos_fov = RvR_fix22_mul(cos,fovx);

   RvR_array_length_set(sector_stack,0);
   RvR_array_push(sector_stack,start);

   port_map->sectors[start].visited = 1;
   while(RvR_array_length(sector_stack)>0)
   {
      int16_t sector = sector_stack[RvR_array_length(sector_stack)-1];
      RvR_array_length_set(sector_stack,RvR_array_length(sector_stack)-1);

      for(int i = 0;i<port_map->sectors[sector].wall_count;i++)
      {
         RvR_port_wall *w0 = port_map->walls+port_map->sectors[sector].wall_first+i;
         RvR_port_wall *w1 = port_map->walls+w0->p2;
         RvR_port_dwall dw = {0};

         //Translate to camera space
         RvR_fix22 x0 = w0->x-port_cam->x;
         RvR_fix22 y0 = w0->y-port_cam->y;
         RvR_fix22 x1 = w1->x-port_cam->x;
         RvR_fix22 y1 = w1->y-port_cam->y;

         //Rotate to camera space
         //tp0y and tp1y are depth afterwards
         //Could cache here, but I don't think it's worth it
         //TODO(Captain4LK): reuse x0,y0,x1,y1 once the render has been checked properly
         RvR_fix22 tp0x = RvR_fix22_mul(-x0,sin)+RvR_fix22_mul(y0,cos);
         RvR_fix22 tp0y = RvR_fix22_mul(x0,cos_fov)+RvR_fix22_mul(y0,sin_fov);
         RvR_fix22 tp1x = RvR_fix22_mul(-x1,sin)+RvR_fix22_mul(y1,cos);
         RvR_fix22 tp1y = RvR_fix22_mul(x1,cos_fov)+RvR_fix22_mul(y1,sin_fov);
         dw.xw0 = tp0x;
         dw.xw1 = tp1x;
         dw.zw0 = tp0y;
         dw.zw1 = tp1y;

         //Add portals if wall is very slim
         //The 2d cross product is used to check how close the angles of the two lines
         //are. If they are very similar, the value goes towards zero.
         //After the z-division these lines end up very thin, so they might not be
         //rendered at all. The compare with the length of the lines is
         //basically just arbitrary, short lines are less likely to produce rendering artifacts
         //TODO(Captain4LK): recheck this
         //TODO(Captain4LK): maybe don't consider lines outside of view frustum?
         int16_t portal = w0->portal;
         if(portal>=0&&!port_map->sectors[portal].visited)
         {
            int64_t cross = (int64_t)tp0x*tp1y-(int64_t)tp1x*tp0y;
            int64_t len = ((int64_t)tp1x-tp0x)*((int64_t)tp1x-tp0x);
            len+=((int64_t)tp1y-tp0y)*((int64_t)tp1y-tp0y);

            if((cross*cross)/256<=len)
            {
               RvR_array_push(sector_stack,portal);
               port_map->sectors[portal].visited = 1;
            }
         }

         //Behind camera
         if(tp0y<=0&&tp1y<=0)
            continue;

         //Not facing camera --> winding
         //if 2d cross product is less than zero, p0 is to the left of p1
         if(RvR_fix22_mul(tp0x,tp1y)-RvR_fix22_mul(tp1x,tp0y)>0)
            continue;

         //Calculate texture coordinates based on line length
         dw.u0 = 0;
         dw.u1 = RvR_fix22_sqrt(RvR_fix22_mul(w1->x-w0->x,w1->x-w0->x)+RvR_fix22_mul(w1->y-w0->y,w1->y-w0->y))*64-1;

         //Clipping
         //Left point in fov
         if(tp0x>=-tp0y)
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            //Simply divide by z 
            dw.x0 = RvR_min(RvR_xres()*512+RvR_fix22_div(tp0x*(RvR_xres()/2),RvR_non_zero(tp0y)),RvR_xres()*1024-1);
            dw.z0 = tp0y;
         }
         //Left point to the left of fov
         else
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            //Calculate intersection of line with frustum
            //and adjust texture and depth acordingly
            dw.x0 = 0;
            RvR_fix22 dx0 = tp1x-tp0x;
            RvR_fix22 dx1 = tp0x+tp0y;
            dw.u0 = dw.u0 + RvR_fix22_div(RvR_fix22_mul(-tp0x-tp0y,dw.u1-dw.u0),RvR_non_zero(tp1x-tp0x+tp1y-tp0y));
            dw.z0 = RvR_fix22_div(RvR_fix22_mul(dx0,dx1),RvR_non_zero(tp1y-tp0y+tp1x-tp0x))-tp0x;
         }

         //Right point in fov
         if(tp1x<=tp1y)
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            //Simply divide by z 
            dw.x1 = RvR_min(RvR_xres()*512+RvR_fix22_div(tp1x*(RvR_xres()/2),RvR_non_zero(tp1y)),RvR_xres()*1024);
            dw.z1 = tp1y;
         }
         else
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            //Calculate intersection of line with frustum
            //and adjust texture and depth acordingly
            RvR_fix22 dx0 = tp1x-tp0x;
            RvR_fix22 dx1 = tp0y-tp0x;
            dw.x1 = RvR_xres()*1024;
            dw.z1 = tp0x-RvR_fix22_div(RvR_fix22_mul(dx0,dx1),RvR_non_zero(tp1y-tp0y-tp1x+tp0x));
            dw.u1 = RvR_fix22_div(RvR_fix22_mul(dx1,dw.u1),RvR_non_zero(-tp1y+tp0y+tp1x-tp0x));
         }

         //If the wall somehow ends up having a
         //negativ width, skip it
         if(dw.x0>dw.x1)
            continue;

         //Near clip
         if(dw.z0<=0||dw.z1<1)
            continue;

         dw.wall = port_map->sectors[sector].wall_first+(uint16_t)i;
         dw.sector = sector;
         RvR_array_push(dwalls,dw);
      }
   }
}
//-------------------------------------
