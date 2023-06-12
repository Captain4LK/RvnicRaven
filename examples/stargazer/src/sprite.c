/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   RvR_fix16 depth;
   uint16_t texture;
   uint16_t flags;
}Sprite_draw;
//-------------------------------------

//Variables
struct
{
   Sprite_draw * restrict data;
   uint_fast16_t * restrict data_proxy;
   uint32_t data_used;
   uint32_t data_size;
} sprite_stack = {0};

static RvR_fix16 view_fov_factor_x;
static RvR_fix16 view_fov_factor_y;
static RvR_fix16 view_sin;
static RvR_fix16 view_cos;
static RvR_fix16 view_sin_fov;
static RvR_fix16 view_cos_fov;
static RvR_fix16 view_middle_row;

Sprite sprites[1 << 16];
//-------------------------------------

//Function prototypes
static void sprite_stack_push(const Sprite_draw *s);

static int sprite_cmp(const void *a, const void *b);
//-------------------------------------

//Function implementations

void sprites_init(void)
{
   //Default settings:
   //   * no rotation
   //   * texture and index are the same
   //   * no flags
   for(int i = 0; i<1 << 16; i++)
   {
      sprites[i].rot = 0;
      sprites[i].tex[0] = i;
      sprites[i].flag[0] = 0;
   }
}

void sprite_draw_begin()
{
   sprite_stack.data_used = 0;
}

void sprite_draw(RvR_fix16 x, RvR_fix16 y, RvR_fix16 z, RvR_fix16 dir, int32_t sprite)
{
   if(sprite<0||sprite>UINT16_MAX)
      return;

   const Sprite *sp = sprites + sprite;
   RvR_ray_draw_sprite(&player.cam, x, y, z, dir, sp->tex[0], 0);
}

void sprite_draw_end()
{}

static void sprite_stack_push(const Sprite_draw *s)
{
   if(sprite_stack.data==NULL)
   {
      sprite_stack.data_size = 64;
      sprite_stack.data = RvR_malloc(sizeof(*sprite_stack.data) * sprite_stack.data_size, "Sprite stack");
      sprite_stack.data_proxy = RvR_malloc(sizeof(*sprite_stack.data_proxy) * sprite_stack.data_size, "Sprite stack proxy");
   }

   sprite_stack.data_proxy[sprite_stack.data_used] = sprite_stack.data_used;
   sprite_stack.data[sprite_stack.data_used++] = *s;

   if(sprite_stack.data_used==sprite_stack.data_size)
   {
      sprite_stack.data_size += 64;
      sprite_stack.data = RvR_realloc(sprite_stack.data, sizeof(*sprite_stack.data) * sprite_stack.data_size, "Sprite stack data grow");
      sprite_stack.data_proxy = RvR_realloc(sprite_stack.data_proxy, sizeof(*sprite_stack.data_proxy) * sprite_stack.data_size, "Sprite stack data_proxy grow");
   }
}

static int sprite_cmp(const void *a, const void *b)
{
   const Sprite_draw *sa = a;
   const Sprite_draw *sb = b;

   return sb->depth - sa->depth;
}
//-------------------------------------
