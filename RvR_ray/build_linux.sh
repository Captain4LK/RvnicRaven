#!/bin/sh
set -e

CFLAGS="-Wall -Wextra -Wshadow -std=c99 -pedantic -Wno-sign-compare -Og -g -I./ -I../include/"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
all: RvR_ray
" "$CFLAGS"

obj=""
for src in $(find ./ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   obj="$obj ${src%%.c}.o"
done
echo "obj =$obj"

printf "RvR_ray: \$(obj)\n\tar crs libRvR_ray.a $^\nclean:\n\trm -f \$(obj) libRvR_ray.a\n"

