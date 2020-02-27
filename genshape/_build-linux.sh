#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu99 -Winline -Wno-write-strings -Wno-format-security -fms-extensions"
L_FLAGS=""

gcc $C_FLAGS -I. -Iinclude genshape.c -L. $L_FLAGS -l:freetype-linux.a -o genshape 
