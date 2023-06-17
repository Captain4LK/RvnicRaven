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
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "card_text.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static const char *card_heart_title[10] =
{
   "Ace of Hearts",
   "Two of Hearts",
   "Three of Hearts",
   "Four of Hearts",
   "Five of Hearts",
   "Six of Hearts",
   "Seven of Hearts",
   "Eight of Hearts",
   "Nine of Hearts",
   "Ten of Hearts",
};
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

const char *card_title(const Card *c)
{
   if(c==NULL||c->type==CARD_NONE)
      return "INVALID CARD";

   switch(c->type)
   {
   case CARD_HEARTS:
      return card_heart_title[(c->rank-1)%10];
   }

   return "INVALID CARD";
}
//-------------------------------------
