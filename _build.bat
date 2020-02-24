@echo off

set C_FLAGS=-g -ggdb -std=gnu99 -Winline -Wno-write-strings -fms-extensions
set L_FLAGS=
set SRC_DIR=%cd%

if not exist .\bin (
  mkdir .\bin
)

pushd .\bin
gcc %C_FLAGS% %SRC_DIR%\win_main.c -lkernel32 -lgdi32 -o drawing.exe
popd
