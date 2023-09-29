#!/bin/sh

cflags="-Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-compare -pedantic -std=c99 -I../../../../include/ -fsanitize=address,undefined"
libs="-lSDL2 -lm ../../../../RvR/libRvR.a"

gcc *.c -o ../../bin/defc $cflags $libs
