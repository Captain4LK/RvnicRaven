#!/bin/sh

x86_64-w64-mingw32-gcc -o texture-tool texture-tool.c @gcc.args -O3 -s -flto=auto -lSDL2 -lmingw32 -lSDL2main -static-libgcc
