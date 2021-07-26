#!/bin/sh

# rm -rf build
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --clean .
cmake --build . -j 8
