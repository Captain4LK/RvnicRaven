/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _PLAYER_H_

#define _PLAYER_H_

typedef struct
{
   Entity *entity;
   RvR_ray_cam cam;

   int16_t shear;
}Player;

extern Player player;

void player_create_new();
void player_update();

#endif
