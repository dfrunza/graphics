@echo off

set C_FLAGS=-g -ggdb -std=gnu99 -Winline -Wno-write-strings -fms-extensions
set L_FLAGS=

gcc %C_FLAGS% win_main.c -lkernel32 -lgdi32 -o drawing.exe
