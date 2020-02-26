#!/bin/bash

SRC_DIR=$PWD
C_FLAGS="-g -ggdb -std=gnu++98 -Winline -Wno-write-strings -fms-extensions"
L_FLAGS=""

mkdir -p ./bin
pushd >/dev/null ./bin
g++ $C_FLAGS -I. -o drawing $SRC_DIR/haiku_main.cpp $L_FLAGS -lbe
popd >/dev/null

