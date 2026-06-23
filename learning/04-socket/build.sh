#!/bin/bash

CMAKE=$(which cmake)
NINJA=$(which ninja)

mkdir -p build-linux
$CMAKE -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=$NINJA -G Ninja -S . -B build-linux

# uncomment for rebuild from scratch
# $CMAKE --build build-linux --target clean -j $(nproc)
$CMAKE --build build-linux --target all -j $(nproc)