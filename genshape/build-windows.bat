@echo off

set C_FLAGS=-g -ggdb -std=gnu99 -Winline -Wno-write-strings -fms-extensions
set L_FLAGS=

gcc %C_FLAGS% -I. -Iinclude genshape.c -L. -l:freetype-windows.a -o genshape.exe 
