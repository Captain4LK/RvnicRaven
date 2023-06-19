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
//-------------------------------------

//Variables
Sprite sprites[1 << 16];
//-------------------------------------

//Function prototypes
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

   //TODO: load from file?

   //Officer
   sprites[16427].rot = 1;
   sprites[16427].tex[0] = 16427;
   sprites[16427].tex[1] = 16428;
   sprites[16427].tex[2] = 16429;
   sprites[16427].tex[3] = 16430;
   sprites[16427].tex[4] = 16431;
   sprites[16427].tex[5] = 16432;
   sprites[16427].tex[6] = 16433;
   sprites[16427].tex[7] = 16434;
}

void sprite_draw_begin()
{
}

void sprite_draw(RvR_fix16 x, RvR_fix16 y, RvR_fix16 z, RvR_fix16 dir, int32_t sprite, void *ref)
{
   if(sprite<0||sprite>UINT16_MAX)
      return;

   const Sprite *sp = sprites + sprite;
   if(!sp->rot)
   {
      RvR_ray_draw_sprite(&player.cam, x, y, z, dir, sp->tex[0], 0,ref);
      return;
   }

   RvR_fix16 rot = RvR_fix16_atan2(player.entity->y-y,player.entity->x-x);
   rot = (rot+4096-dir)&65535;
   RvR_ray_draw_sprite(&player.cam,x,y,z,dir,sp->tex[(rot/8192)&7],0,ref);
}

void sprite_draw_end()
{}
//-------------------------------------
