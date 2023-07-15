/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"

//Sigh...
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif
//-------------------------------------

//Internal includes
#include "util.h"
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

#ifdef _WIN32

void util_mkdir(const char *path)
{
   _mkdir(path);
}

#else

void util_mkdir(const char *path)
{
   mkdir(path, 0755);
}

#endif
//-------------------------------------
