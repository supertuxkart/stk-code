#!/bin/bash
set -ev

mkdir -p build
cd build
cmake ..
make
#make install
