#!/bin/bash

SRC_DIR=$PWD
C_FLAGS="-g -ggdb -std=gnu99 -Winline -Wno-write-strings -fms-extensions"
L_FLAGS=""

mkdir -p ./bin
pushd >/dev/null ./bin
gcc $C_FLAGS -I. -o drawing $SRC_DIR/x11_main.c $L_FLAGS -lxcb -lxcb-icccm -lxcb-image -lm
popd >/dev/null
