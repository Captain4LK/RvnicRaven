#!/bin/sh
set -e

CFLAGS="-Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -std=c99 -pedantic -Og -g3 -I./ -I../include/ -I./include/ -fsanitize=undefined"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
all: RvR 
" "$CFLAGS"

obj=""
for src in $(find ./ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   obj="$obj ${src%%.c}.o"
done
echo "obj =$obj"

printf "RvR: \$(obj)\n\tar crs libRvR.a $^\nclean:\n\trm -f \$(obj) libRvR.a\n"

