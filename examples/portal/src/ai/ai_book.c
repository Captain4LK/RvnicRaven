/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "entity.h"
#include "ai.h"

#include "ai/ai_book.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
}AI_book_state;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void ai_book_run(Entity *e)
{
   e->ai_data = NULL;
   e->radius = 0;
   e->height = 0;
}

void ai_book_free(Entity *e)
{
}

void ai_book_init(Entity *e, const uint32_t args[4])
{
}

void ai_book_on_use(Entity *e, Entity *trigger)
{
}
//-------------------------------------
