#!/bin/sh

gcc -o palette-tool palette-tool.c -I../../external -I../../include ../../RvR/libRvR_sdl2.a -lm -g -O3 -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -std=c99 -pedantic
