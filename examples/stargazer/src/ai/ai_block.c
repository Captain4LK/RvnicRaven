/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "../config.h"
#include "../card.h"
#include "../sprite.h"
#include "../entity.h"
#include "../ai.h"
#include "../game.h"
#include "../player.h"
#include "../grid.h"
#include "ai_block.h"
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

void ai_block_init(Entity *e, const uint32_t extra[3])
{
   e->ai_data = NULL;
   e->sprite = -1;

   e->col_radius = extra[0];
   e->col_height = extra[1];
}

void ai_block_free(Entity *e)
{}

void ai_block_run(Entity *e)
{}
//-------------------------------------
