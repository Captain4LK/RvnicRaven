#!/bin/sh

gcc -o palette-viewer palette-viewer.c -I../../external -I../../include ../../RvR/libRvR_sdl2.a -lSDL2 -lm -g -O3 -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -std=c99 -pedantic
