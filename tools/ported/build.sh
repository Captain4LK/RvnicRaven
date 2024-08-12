#!/bin/sh
set -e

# -isystem to not list all RvR headers
CFLAGS="-Wall -Wextra -Wshadow -std=c99 -Wno-sign-compare -Wconversion -Wno-sign-conversion -O3 -g -I./ -isystem ../../include -I../../external/ -fno-omit-frame-pointer -fsanitize=undefined,address"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
all: ported
.PHONY: ported 
" "$CFLAGS"

obj=""

obj="$obj ../../external/tinyfiledialogs.o" 
for src in $(find ./ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   obj="$obj ${src%%.c}.o"
done

echo "obj= $obj"

printf "ported: \$(obj)\n\tgcc -o ./ported $^ $CFLAGS ../../RvR_portal/libRvR_port.a ../../RvR/libRvR_gui.a -lSDL2 -lm\nclean:\n\trm -f \$(obj) ./ported\n"
