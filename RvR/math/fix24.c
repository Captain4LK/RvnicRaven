/*
RvnicRaven - 24.8 fixed point math

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
//-------------------------------------

//Internal includes
#include "RvR/RvR_math.h"
#include "RvR/RvR_fix24.h"
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

RvR_fix24 RvR_fix24_mul(RvR_fix24 a, RvR_fix24 b)
{
   return (RvR_fix24)((int64_t)a * b >> 8);
}

RvR_fix24 RvR_fix24_div(RvR_fix24 a, RvR_fix24 b)
{
   return (RvR_fix24)(((int64_t)a << 8) / b);
}
//-------------------------------------
