#!/bin/sh

gcc -o ../bin/iso *.c ../../../RvR_ray/libRvR_ray.a ../../../RvR/libRvR.a -std=c99 -I ../../../include/ -Wall -Wextra -Wshadow -Wno-sign-compare -pedantic -lSDL2 -lSDL2_mixer -O3 -g -lm  -lprofiler -fsanitize=address
