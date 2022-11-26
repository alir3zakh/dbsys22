#!/bin/sh

# automated script to make ninja build files and compiling afterwards.

mkdir -p build/debug;
cd build/debug;

cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug ../.. && ninja;
