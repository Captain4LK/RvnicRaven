/*
RvnicRaven - stargazer

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "game.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
#include "player_weapon.h"
#include "collision.h"
#include "grid.h"
#include "state.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void player_weapon_update(Player *p)
{
   if(p==NULL)
      return;

   switch(p->wstate.state)
   {
   case P_WEAPON_NONE:
      break;
   case P_WEAPON_SWITCH:
      break;
   }
}

void player_weapon_switch(Player *p, int card_slot)
{
   if(p==NULL)
      return;

   //if(weapon==P_WEAPON_NONE||weapon==P_WEAPON_SWITCH)
   //return;

   p->wstate.as.wswitch.from = p->wstate.state;
   //p->wstate.as.wswitch.to = weapon;
   p->wstate.state = P_WEAPON_SWITCH;
   p->wstate.substate = 0;
}
//-------------------------------------
