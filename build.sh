#!/bin/sh

# rm -rf build
mkdir -p build
cd build
cmake ..
cmake --build . --target clean
cmake --build . -j 8
