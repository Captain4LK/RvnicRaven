#!/bin/sh

gcc -o texture-tool texture-tool.c -I../../external -I../../include ../../RvR/libRvR_sdl2.a -lSDL2 -lm -g -Og -Wall -Wextra
