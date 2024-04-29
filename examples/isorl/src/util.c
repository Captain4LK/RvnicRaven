/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//Sigh...
#ifdef _WIN32
#include <windows.h>
#else
#define _POSIX_C_SOURCE 200112L
#include <unistd.h>
#include <sys/stat.h>
#endif

//External includes
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "RvR/RvR.h"
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

int util_mkdir(const char *path)
{
   int res = _mkdir(path);
   if(res==-1)
   {
      if(errno==EEXIST)
         return 1;
      return -1;
   }

   return 0;
}

int util_truncate(RvR_rw *rw, int32_t len)
{
   if(rw==NULL||(rw->type!=RVR_RW_STD_FILE&&rw->type!=RVR_RW_STD_FILE_PATH))
      return 0;

   return _chsize(_fileno(rw->as.fp), len);
}

#else

int util_mkdir(const char *path)
{
   int res = mkdir(path, 0755);
   if(res==-1)
   {
      if(errno==EEXIST)
         return 1;
      return -1;
   }

   return 0;
}

int util_truncate(RvR_rw *rw, int32_t len)
{
   if(rw==NULL||(rw->type!=RVR_RW_STD_FILE&&rw->type!=RVR_RW_STD_FILE_PATH))
      return 0;

   return ftruncate(fileno(rw->as.fp), len);
}

#endif
//-------------------------------------
