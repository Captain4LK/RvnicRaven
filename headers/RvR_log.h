#ifndef _RVR_LOG_H_

/*
   RvnicRaven - logging

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/*
Needs:
   <stdio.h>
*/

#define _RVR_LOG_H_

#define RvR_log(...) fprintf(stderr,__VA_ARGS__);

#define RvR_log_line(w,...) do { char RvR_log_line_tmp[1024]; snprintf(RvR_log_line_tmp,1024,__VA_ARGS__); RvR_log(w " (%s:%u): %s\n",__FILE__,__LINE__,RvR_log_line_tmp); } while(0)

#define RvR_error_fail(w,...) do { RvR_log_line(w,__VA_ARGS__); goto RvR_err; } while(0)

#define RvR_error_check(X,w,...) do { if(!(X)) RvR_error_fail(w,__VA_ARGS__); } while(0)

#endif
