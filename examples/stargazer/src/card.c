/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Card *cards = NULL;

static Card *card_pool = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Card *card_new()
{
   if(card_pool==NULL)
   {
      Card *nc = RvR_malloc(sizeof(*nc) * 256, "Card pool");
      memset(nc, 0, sizeof(*nc) * 256);

      for(int i = 0; i<256 - 1; i++)
         nc[i].next = &nc[i + 1];
      card_pool = &nc[0];
   }

   Card *n = card_pool;
   card_pool = n->next;
   n->next = NULL;
   n->prev_next = NULL;

   memset(n, 0, sizeof(*n));

   return n;
}

void card_free(Card *c)
{
   if(c==NULL)
      return;

   *c->prev_next = c->next;
   if(c->next!=NULL)
      c->next->prev_next = c->prev_next;

   c->next = card_pool;
   card_pool = c;
}

void card_add(Card *c)
{
   if(c==NULL)
      return;

   c->prev_next = &cards;
   if(cards!=NULL)
      cards->prev_next = &c->next;
   c->next = cards;
   cards = c;
}

void card_remove(Card *c)
{
   if(c==NULL)
      return;

   c->removed = 1;
}

void card_copy(Card *dst, const Card *src)
{
   dst->type = src->type;
   dst->rank = src->rank;
   dst->durability = src->durability;
   dst->x = src->x;
   dst->y = src->y;
   dst->z = src->z;
}

void card_health(Card *c, int health)
{
   if(c==NULL)
      return;

   c->type = CARD_HEARTS;
   c->rank = health;
}
//-------------------------------------
