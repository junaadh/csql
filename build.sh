#!/bin/sh

set -xe

mkdir -p build
CFLAGS="-Wall -Wextra"
CLIBS="-lm"
PROGRAM="csql"

clang $CFLAGS -o "build/$PROGRAM" "src/main.c" "src/$PROGRAM.c" $CLIBS
