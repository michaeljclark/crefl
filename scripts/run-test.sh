#!/bin/bash

# output reflection metadata to file
test -d build/tmp || mkdir -p build/tmp
clang $1 -Xclang -load \
         -Xclang build/libcrefl.so \
         -Xclang -plugin -Xclang crefl \
         -Xclang -plugin-arg-crefl -Xclang -o \
         -Xclang -plugin-arg-crefl -Xclang build/tmp/$(basename $1).refl

# dump reflection metadata from file
./build/crefltool build/tmp/$(basename $1).refl

# dump the original source
cat $1
