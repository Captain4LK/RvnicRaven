#!/bin/sh

gcc -o ../../bin/vm main.c @gcc.args -O3 -flto
