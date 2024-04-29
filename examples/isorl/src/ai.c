/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "ai.h"
#include "action.h"
#include "player.h"
#include "spiral_path.h"
#include "astar.h"
#include "point.h"
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

void ai_think(World *w, Area *a, Entity *e)
{
   FOV_object *objects = fov_entity(a, e);

   for(int i = 0; i<RvR_array_length(objects); i++)
   {
      if(objects[i].type==FOV_OBJECT_ENTITY&&objects[i].as.e==player.e)
      {
         if(point_adjacent(e->pos, objects[i].as.e->pos))
            action_set_attack(e, point_dir(e->pos, objects[i].as.e->pos));
         else
            action_set_path(a, e, objects[i].as.e->pos, 0);
      }
   }
}
//-------------------------------------
