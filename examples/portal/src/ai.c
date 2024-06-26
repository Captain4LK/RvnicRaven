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

#include "ai/ai_none.h"
#include "ai/ai_item.h"
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

void ai_run(Entity *e)
{
}

void ai_free(Entity *e)
{
}

void ai_init(Entity *e, uint32_t ai_type, const uint32_t args[4])
{
   switch(ai_type)
   {
   case AI_NONE: return;
   case AI_ITEM: return;
   }
}

void ai_on_use(Entity *e, Entity *trigger)
{
}

void ai_on_touch(Entity *e, Entity *trigger)
{
}
//-------------------------------------
