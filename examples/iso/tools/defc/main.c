/*
RvnicRaven - iso roguelike: definition compiler

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <RvR/RvR.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "parse.h"
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

int main(int argc, char **argv)
{
   Parser parser = {0};
   parser_init(&parser,"out.bdef");
   parse_file(&parser,"test.ini");
   parser_close(&parser);

   return 0;
}
//-------------------------------------
