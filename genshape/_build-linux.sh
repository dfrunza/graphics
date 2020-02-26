#!/bin/bash

SRC_DIR=$PWD
BIN_DIR=.
C_FLAGS="-g -ggdb -std=gnu99 -Winline -Wno-write-strings -fms-extensions"
L_FLAGS=""

gcc $C_FLAGS -I. -I./include -o genshape $SRC_DIR/genshape.c -L. $L_FLAGS -l:./freetype-linux.a
