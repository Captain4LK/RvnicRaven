#!/bin/sh

gcc -o ../../bin/graphics-builder *.c -I../../../../external -I../../../../include ../../../../RvR/libRvR_sdl2.a -lSDL2 -lm -g -Og -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -std=c99 -pedantic -fsanitize=undefined,address
