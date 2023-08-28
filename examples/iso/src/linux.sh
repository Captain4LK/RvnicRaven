#!/bin/sh

WARNINGS="-Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-compare -pedantic"
echo $WARNINGS

gcc -o ../bin/iso *.c ../../../RvR_ray/libRvR_ray.a ../../../RvR/libRvR.a -std=c99 -I ../../../include/ $WARNINGS -lSDL2 -lSDL2_mixer -O3 -g -lm  -lprofiler
