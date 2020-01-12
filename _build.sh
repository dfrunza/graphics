SRC_DIR=$PWD
C_FLAGS="-std=gnu99 -ggdb -Winline -Wno-write-strings"
L_FLAGS=""

pushd >/dev/null ./bin
gcc $C_FLAGS -I. -o drawing $SRC_DIR/drawing.c $L_FLAGS -lxcb -lxcb-icccm -lxcb-image -lm
popd >/dev/null
