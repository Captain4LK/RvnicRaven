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
      body->parts[i].hp_max = 50;

      int slot_count = 0;
      if(body->parts[i].def->tags&DEF_BODY_SLOT_UPPER) slot_count+=4;
      if(body->parts[i].def->tags&DEF_BODY_SLOT_LOWER) slot_count+=3;
      if(body->parts[i].def->tags&DEF_BODY_SLOT_HEAD) slot_count+=3;
      if(body->parts[i].def->tags&DEF_BODY_SLOT_HAND) slot_count+=3;
      if(body->parts[i].def->tags&DEF_BODY_SLOT_FOOT) slot_count+=3;

      if(slot_count>0)
      {
         body->parts[i].slots = RvR_malloc(sizeof(*body->parts[i].slots)*slot_count,"Body part item slots");
         body->parts[i].slot_count = slot_count;

         int cur = 0;
         if(body->parts[i].def->tags&DEF_BODY_SLOT_UPPER)
         {
            body->parts[i].slots[cur].type = ITEM_SLOT_UPPER;
            body->parts[i].slots[cur].layer = ITEM_SLOT_UNDER;
            body->parts[i].slots[cur+1].type = ITEM_SLOT_UPPER;
            body->parts[i].slots[cur+1].layer = ITEM_SLOT_OVER;
            body->parts[i].slots[cur+2].type = ITEM_SLOT_UPPER;
            body->parts[i].slots[cur+2].layer = ITEM_SLOT_ARMOR;
            body->parts[i].slots[cur+3].type = ITEM_SLOT_UPPER;
            body->parts[i].slots[cur+3].layer = ITEM_SLOT_BACK;
            cur+=4;
         }
         if(body->parts[i].def->tags&DEF_BODY_SLOT_LOWER)
         {
            body->parts[i].slots[cur].type = ITEM_SLOT_LOWER;
            body->parts[i].slots[cur].layer = ITEM_SLOT_UNDER;
            body->parts[i].slots[cur+1].type = ITEM_SLOT_LOWER;
            body->parts[i].slots[cur+1].layer = ITEM_SLOT_OVER;
            body->parts[i].slots[cur+2].type = ITEM_SLOT_LOWER;
            body->parts[i].slots[cur+2].layer = ITEM_SLOT_ARMOR;
            cur+=3;
         }
         if(body->parts[i].def->tags&DEF_BODY_SLOT_HEAD)
         {
            body->parts[i].slots[cur].type = ITEM_SLOT_HEAD;
            body->parts[i].slots[cur].layer = ITEM_SLOT_UNDER;
            body->parts[i].slots[cur+1].type = ITEM_SLOT_HEAD;
            body->parts[i].slots[cur+1].layer = ITEM_SLOT_OVER;
            body->parts[i].slots[cur+2].type = ITEM_SLOT_HEAD;
            body->parts[i].slots[cur+2].layer = ITEM_SLOT_ARMOR;
            cur+=3;
         }
         if(body->parts[i].def->tags&DEF_BODY_SLOT_HAND)
         {
            body->parts[i].slots[cur].type = ITEM_SLOT_HAND;
            body->parts[i].slots[cur].layer = ITEM_SLOT_UNDER;
            body->parts[i].slots[cur+1].type = ITEM_SLOT_HAND;
            body->parts[i].slots[cur+1].layer = ITEM_SLOT_OVER;
            body->parts[i].slots[cur+2].type = ITEM_SLOT_HAND;
            body->parts[i].slots[cur+2].layer = ITEM_SLOT_ARMOR;
            cur+=3;
         }
         if(body->parts[i].def->tags&DEF_BODY_SLOT_FOOT)
         {
            body->parts[i].slots[cur].type = ITEM_SLOT_FOOT;
            body->parts[i].slots[cur].layer = ITEM_SLOT_UNDER;
            body->parts[i].slots[cur+1].type = ITEM_SLOT_FOOT;
            body->parts[i].slots[cur+1].layer = ITEM_SLOT_OVER;
            body->parts[i].slots[cur+2].type = ITEM_SLOT_FOOT;
            body->parts[i].slots[cur+2].layer = ITEM_SLOT_ARMOR;
            cur+=3;
         }
      }
   }
}
//-------------------------------------
