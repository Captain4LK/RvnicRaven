#!/bin/sh

gcc -o texture-tool texture-tool.c @gcc.args -Og -g -lSDL2 -lm
