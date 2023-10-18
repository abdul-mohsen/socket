#!/bin/bash
set -ex
clear
rm -rf build
mkdir build

clang -O3 -Wall -Wextra -o build/server server.c -ggdb
clang -O3 -Wall -Wextra -o build/client client.c -ggdb

./build/server 
echo done
