#!/bin/sh
set -e

# -isystem to not list all RvR headers
CFLAGS="-Wall -Wextra -Wshadow -std=c99 -Wno-sign-compare -Wconversion -Wno-sign-conversion -O3 -g -I./ -isystem ../../../include -lprofiler -fsanitize=undefined"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
all: port
" "$CFLAGS"

obj=""

for src in $(find ./ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   obj="$obj ${src%%.c}.o"
done

echo "obj= $obj"

printf "port: \$(obj)\n\tgcc -o ../bin/port $^ $CFLAGS ../../../RvR_portal/libRvR_port.a ../../../RvR/libRvR_sdl2.a -lSDL2\nclean:\n\trm -f \$(obj) ../bin/port\n"
