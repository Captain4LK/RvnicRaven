/*
RvnicRaven - stargazer

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _PLAYER_WEAPON_H_

#define _PLAYER_WEAPON_H_

typedef enum
{
   P_WEAPON_NONE,
   P_WEAPON_SWITCH,

   P_WEAPON_SLOT_1,
   P_WEAPON_SLOT_2,
   P_WEAPON_SLOT_3,
   P_WEAPON_SLOT_4,
   P_WEAPON_SLOT_5,
   P_WEAPON_SLOT_6,
   P_WEAPON_SLOT_7,
   P_WEAPON_SLOT_8,
   P_WEAPON_SLOT_9,
}P_weapon_statenum;

typedef struct
{
   P_weapon_statenum state;
   int current;
   uint8_t substate;

   union
   {
      struct
      {
         P_weapon_statenum from;
         P_weapon_statenum to;
      }wswitch;
   }as;
}P_weapon_state;

void player_weapon_update(Player *p);
void player_weapon_switch(Player *p, int card_slot);

#endif
