/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "../../RvR/RvnicRaven.h"
//-------------------------------------

//Internal includes
#include "../config.h"
#include "../card.h"
#include "../sprite.h"
#include "../entity.h"
#include "../ai.h"
#include "../game.h"
#include "../player.h"
#include "../grid.h"
#include "ai_deco.h"
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

void ai_deco_init(Entity *e, const uint32_t extra[3])
{
   e->ai_data = NULL;

   e->col_radius = 372;
   e->col_height = 768;
}

void ai_deco_free(Entity *e)
{
}

void ai_deco_run(Entity *e)
{
}
//-------------------------------------
