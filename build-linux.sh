#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu99 -Winline -Wno-write-strings -fms-extensions"
L_FLAGS=""

gcc $C_FLAGS -I. -o drawing x11_main.c $L_FLAGS -lxcb -lxcb-icccm -lxcb-image -lm
