#!/bin/sh

i686-w64-mingw32-gcc -o texture-tool texture-tool.c -I../../external -I../../include ../../RvR_ray/libRvR_ray.a ../../RvR/libRvR.a -lSDL2 -lm -g -Og -Wall -Wextra -lmingw32 -lSDL2main -static-libgcc
