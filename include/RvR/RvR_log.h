/*
RvnicRaven - logging

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_LOG_H_

#define _RVR_LOG_H_

void RvR_log(const char *format, ...);
void RvR_logl(const char *fun, const char *file, unsigned line, const char *format, ...);

#define RvR_log_line(w,...) RvR_logl(w,__FILE__,__LINE__,__VA_ARGS__)

#define RvR_error_fail(w, ...) do { RvR_log_line(w, __VA_ARGS__); goto RvR_err; } while(0)

#define RvR_error_check(X, w, ...) do { if(!(X)) RvR_error_fail(w, __VA_ARGS__); } while(0)

#endif
