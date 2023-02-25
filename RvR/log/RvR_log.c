/*
RvnicRaven - logging

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdarg.h>
//-------------------------------------

//Internal includes
#include "RvR/RvR_log.h"
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

void RvR_log(const char *format, ...)
{
   va_list args;
   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
}

void RvR_logl(const char *fun, const char *file, unsigned line, const char *format, ...)
{
   fprintf(stderr, "%s (%s:%u): ", fun, file, line);

   va_list args;
   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
}
//-------------------------------------
