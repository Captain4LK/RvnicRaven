/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _STATE_H_

#define _STATE_H_

typedef enum
{
   STATE_TITLE,

   STATE_GAME,

   STATE_GAME_CARD,
   STATE_GAME_CARD_VIEW,
   STATE_GAME_CARD_INVENTORY,

   STATE_GAME_INVENTORY,
}State;

void state_set(State s);
void state_init(State s);
void state_draw();
void state_update();

#endif
