#!/bin/sh
set -e

# -isystem to not list all RvR headers
CFLAGS="-Wall -Wextra -Wshadow -std=c99 -Wno-sign-compare -Wconversion -Wno-sign-conversion -O3 -g -I./ -isystem ../../../../include -I../../../../external/ -lprofiler -fsanitize=undefined,address"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
all: book-tool
" "$CFLAGS"

obj=""

for src in $(find ./ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   obj="$obj ${src%%.c}.o"
done

echo "obj= $obj"

printf "book-tool: \$(obj)\n\tgcc -o ./book-tool $^ $CFLAGS ../../../../RvR/libRvR_sdl2.a -lSDL2\nclean:\n\trm -f \$(obj) ./book-tool \n"

