#!/bin/sh

gcc -o palette-tool palette-tool.c @gcc.args -O3 -flto=auto -s -lm
