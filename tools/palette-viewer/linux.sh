#!/bin/sh

gcc -o palette-viewer palette-viewer.c -I../../external -I../../include ../../RvR/libRvR_sdl2.a -lSDL2 -lm -g -Og -Wall -Wextra
