#!/bin/bash

C_FLAGS="-g -ggdb -std=gnu++98 -Winline -Wno-write-strings -fms-extensions"
L_FLAGS=""

g++ $C_FLAGS -I. -o drawing haiku_main.cpp $L_FLAGS -lbe

