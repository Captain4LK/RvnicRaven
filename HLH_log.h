#ifndef _HLH_LOG_H_

/*
   logging, I guess

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/* 
   To create implementation (the function definitions) add
      #define HLH_LOG_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _HLH_LOG_H_

#define HLH_log_line(w,...) do { char HLH_log_line_tmp[1024]; snprintf(HLH_log_line_tmp,1024,__VA_ARGS__); fprintf(stderr,w " (%s:%u): %s\n",__FILE__,__LINE__,HLH_log_line_tmp); } while(0)

#define HLH_error_fail(w,...) do { HLH_log_line(w,__VA_ARGS__); goto HLH_err; } while(0)

#define HLH_error_check(X,w,...) do { if(!(X)) HLH_error_fail(w,__VA_ARGS__); } while(0)

#endif
