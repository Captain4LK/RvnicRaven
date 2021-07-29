#ifndef _HLH_ERROR_H_

/*
   HLH error reporting extension

   Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/* 
   To create implementation (the function definitions) add
      #define HLH_ERROR_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _HLH_ERROR_H_

typedef enum
{
   HLH_ERROR_NONE = 0,
}HLH_error;

void HLH_error_set(const char *file, int line, int reason);

HLH_error HLH_error_get();

//Returns the last error as a string, 
//returned string will only be valid until next HLH_error_get_string() call
const char *HLH_error_get_string();

#undef HLH_ERROR_FAIL
#undef HLH_ERROR_CHECK

#define HLH_ERROR_FAIL(X) do { HLH_set_error(__FILE__,__LINE__,(X)); goto HLH_err; } while(0)

#define HLH_ERROR_CHECK(X,Y) do { if(!(X)) HLH_ERROR_FAIL(Y); } while(0)

#endif

#ifdef HLH_ERROR_IMPLEMENTATION
#ifndef HLH_ERROR_IMPLEMENTATION_ONCE
#define HLH_ERROR_IMPLEMENTATION_ONCE

#include <stdio.h>
#include <string.h>

static const char *HLH_error_file = NULL;
static int HLH_error_line = 0;
static HLH_error HLH_error_reason = 0;
static char HLH_error_string[512];

void HLH_error_set(const char *file, int line, int reason)
{
   HLH_error_file = file;
   HLH_error_line = line;

   switch(reason)
   {
   case 0: HLH_error_reason = HLH_ERROR_NONE; break;
   }
}

HLH_error HLH_error_get()
{
   return HLH_error_reason;
}

const char *HLH_error_get_string()
{
   if(HLH_error_reason==HLH_ERROR_NONE)
   {
      strncpy(HLH_error_string,"No error encountered",512);
      return HLH_error_string;
   }

   const char *desc = "(NULL)";
   switch(HLH_error_reason)
   {
   case HLH_ERROR_NONE: break; //Handled above
   }

   snprintf(HLH_error_string,512,"(%s:%d): %s",HLH_error_file,HLH_error_line,desc);
   return HLH_error_string;
}

#endif
#endif
