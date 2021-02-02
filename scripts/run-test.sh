#!/bin/bash
clang $1 -Xclang -load \
         -Xclang build/libcrefl.so \
         -Xclang -plugin -Xclang crefl
cat $1
