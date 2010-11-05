#!/bin/sh

echo "Generating ./configure script"
./autogen.sh

echo "Configuring"
./configure

echo "Make"
make  | tee config.errors
