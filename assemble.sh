#!/bin/bash

as -g $1 -o $1.o
ld $1.o -o $1 -e _main -lc /usr/lib/libc.so.6
rm $1.o
