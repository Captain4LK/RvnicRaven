#!/bin/sh

gcc -o build-tool build-tool.c @gcc.args -Og -g -lSDL2 -lm
