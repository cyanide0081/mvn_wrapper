#!/bin/sh
# supports Clang and GCC

gcc --version > /dev/null 2>&1 && CC="gcc"
# clang --version > /dev/null 2>&1 && CC="clang"

if [ "$CC" = "" ]; then
    echo "no suitable compiler found"
    exit 1
fi

FLAGS="-o mvn -std=c99 -Wall -Wextra -Wpedantic"
LFLAGS="-D_GNU_SOURCE -Wl,-u,PROGRAM_NAME"
if [ "$1" = "debug" ]; then
    DFLAGS="-O0 -g -ggdb"
else
    DFLAGS="-Os -DNDEBUG"
fi

set -x
$CC src/mvn.c $FLAGS $DFLAGS $LFLAGS || exit 1
