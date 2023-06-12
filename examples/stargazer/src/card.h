/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _CARD_H_

#define _CARD_H_

typedef enum
{
   CARD_NONE = -1,
   CARD_HEARTS = 0,
}Card_type;

typedef struct Card Card;

struct Card
{
   Card_type type;

   int rank;
   int durability;

   int removed;

   //World card
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   Card *next;
   Card **prev_next;
   //-------------------------------------
};

extern Card *cards;

Card *card_new();
void card_free(Card *c);
void card_add(Card *c);
void card_remove(Card *c);

void card_copy(Card *dst, const Card *src);

#endif
