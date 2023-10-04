/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <RvR/RvR.h>
//-------------------------------------

//Internal includes
#include "body.h"
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

void body_from_def(Body *body, const BodyDef *def)
{
   body->def = def;
   body->part_count = def->bodypart_count;
   body->parts = RvR_malloc(sizeof(*body->parts)*body->part_count,"Body parts array");
   for(int i = 0;i<body->part_count;i++)
   {
      body->parts[i].def = &def->bodyparts[i];
      body->parts[i].next = def->bodyparts[i].next;
      body->parts[i].child = def->bodyparts[i].child;
      body->parts[i].hp = 50;
   }
}
//-------------------------------------
