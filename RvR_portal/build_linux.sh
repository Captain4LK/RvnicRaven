#!/bin/sh
set -e

CFLAGS="-Wall -Wextra -Wshadow -std=c99 -pedantic -Wno-sign-compare -O3 -I./ -I../include/ -g -fno-omit-frame-pointer"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
all: RvR_port
" "$CFLAGS"

obj=""
for src in $(find ./ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   obj="$obj ${src%%.c}.o"
done
echo "obj =$obj"

printf "RvR_port: \$(obj)\n\tar crs libRvR_port.a $^\nclean:\n\trm -f \$(obj) libRvR.a\n"

