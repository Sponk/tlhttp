#!/bin/sh -e

clang -fsanitize=fuzzer,address -std=c++11 fuzz.cpp ../src/*.cpp -I../src -o fuzz $(pkg-config --cflags openssl) $(pkg-config --libs openssl)
./fuzz
