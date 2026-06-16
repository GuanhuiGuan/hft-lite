#!/bin/bash

CMAKE=$(which cmake)
NINJA=$(which ninja)

mkdir -p build-linux
$CMAKE -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=$NINJA -G Ninja -S . -B build-linux

$CMAKE --build build-linux --target clean -j 4
$CMAKE --build build-linux --target all -j 4