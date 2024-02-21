#!/bin/sh
set -e

all_obj=""
add_to_obj()
{
   for src in $(find ./$1 -name "*.c"); do
      gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
      obj="$obj ${src%%.c}.o"
      all_obj="$all_obj ${src%%.c}.o"
   done
}

CFLAGS="-Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -std=c99 -pedantic -O3 -g -I./ -I../include/ -I./include/"
printf "
.POSIX:
CC      = gcc
CFLAGS  = %s
LDFLAGS =
LDLIBS  = 
" "$CFLAGS"

all=""
for src in $(find ./app/ -name "*.c"); do
   all="$all RvR_`basename $src .c`"
done
echo "all: $all"

# Add all source dirs
obj=""
add_to_obj 'clip'
add_to_obj 'compress'
add_to_obj 'config'
add_to_obj 'file'
add_to_obj 'hash'
add_to_obj 'include'
add_to_obj 'log'
add_to_obj 'math'
add_to_obj 'memory'
add_to_obj 'pak'
add_to_obj 'palette'
add_to_obj 'render'
add_to_obj 'texture'
echo "obj =$obj"

#Add target for every source in app/
for src in $(find ./app/ -name "*.c"); do
   gcc -MM -MT "${src%%.c}.o" "$src" $CFLAGS
   target=RvR_`basename $src .c`
   all_obj="$all_obj ${src%%.c}.o"
   printf "$target: \$(obj) ${src%%.c}.o \n\tar crs lib$target.a $^\n"
done

echo "all_obj=$all_obj"

printf "clean:\n\trm -f \$(all_obj) *.a\n"

