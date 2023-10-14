#!/bin/bash
set -ex
clear
rm -rf build
mkdir build

clang -O3 -Wall -Wextra -o build/main main.c -ggdb

./build/main www.example.com
echo done
