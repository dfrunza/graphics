C_FLAGS="-std=gnu99 -ggdb -Winline -Wno-write-strings"
L_FLAGS=""

gcc $C_FLAGS -I. -o drawing drawing.c $L_FLAGS -lxcb -lxcb-icccm -lxcb-image -lm

