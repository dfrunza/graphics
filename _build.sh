C_FLAGS="-std=gnu99 -ggdb -Winline -Wno-write-strings"

gcc $C_FLAGS -o drawing drawing.c -lxcb -lxcb-icccm -lxcb-image -lm

