#!/bin/sh
set -e

CFLAGS="-Wall -Wextra -Wshadow -std=gnu99 -Wno-sign-compare -Wconversion -Wno-sign-conversion -O3 -I./ -I../../../include -g"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
all: iso
" "$CFLAGS"

obj=""

for src in $(find ./ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   obj="$obj ${src%%.c}.o"
done

echo "obj= $obj"

printf "iso: \$(obj)\n\tgcc -o ../bin/iso $^ $CFLAGS ../../../RvR/libRvR_sdl2.a -lSDL2\nclean:\n\trm -f \$(obj) ../bin/iso\n"
