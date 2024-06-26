/*
RvnicRaven - stargazer

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
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
#include "ai_officer.h"
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

void ai_officer_init(Entity *e, const uint32_t extra[3])
{
   e->ai_data = NULL;
   e->sprite = 16427;

   //Inventory
   //--> 50 health?
   e->cards_size = 16;
   e->cards = RvR_malloc(sizeof(*e->cards) * e->cards_size, "AI officer cards");
   memset(e->cards, 0, sizeof(*e->cards) * e->cards_size);

   card_health(&e->cards[0], 10);
   card_health(&e->cards[1], 10);
   card_health(&e->cards[2], 10);
   card_health(&e->cards[3], 10);
   card_health(&e->cards[4], 10);

   e->col_radius = 16384;
   e->col_height = 50162;
}

void ai_officer_free(Entity *e)
{}

void ai_officer_run(Entity *e)
{}
//-------------------------------------
