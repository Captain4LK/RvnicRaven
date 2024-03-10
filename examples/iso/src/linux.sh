#!/bin/sh
set -e

CFLAGS="-Wall -Wextra -Wshadow -std=c99 -pedantic -Wno-sign-compare -Wconversion -Wno-sign-conversion -O3 -I./include/ -I../../../include/ -g -fno-omit-frame-pointer"
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
echo "obj = $obj"

printf "iso: \$(obj)\n\tgcc -o ../bin/iso $^ ../../../RvR/libRvR_sdl2.a -O3 -g -lm -lSDL2 $CFLAGS\nclean:\n\trm -f \$(obj) ../bin/iso\n"


#!/bin/sh

#WARNINGS="-Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-compare -pedantic"
#echo $WARNINGS

#gcc -o ../bin/iso *.c ../../../RvR/libRvR_sdl2.a -std=c99 -I ../../../include/ $WARNINGS -lSDL2 -lSDL2_mixer -O3 -g -lm  -lprofiler
