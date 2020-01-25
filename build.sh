#!/bin/bash
# https://stackoverflow.com/a/28663374/1779853

cython --cplus parallelSort.pyx  
g++  -g -march=native -Ofast -fpic -c    parallelSort.cpp -o parallelSort.o -fopenmp -I$(python -c "import numpy as np; print(np.get_include())") -I"$(find $(dirname $(dirname $(which conda)))/include -maxdepth 1 -iname 'python*' )"
g++  -g -march=native -Ofast -shared  -o parallelSort.so parallelSort.o  -lgomp

# https://stackoverflow.com/a/35317443/1779853
pushd usort/usort
cc -DBUILDING_u8_sort -D__BYTE_ORDER=__LITTLE_ENDIAN -DBUILDING_u4_sort -I/usr/include -I./ -I../ -I../../ -std=c99 -fgnu89-inline -O3 -g -fPIC -shared -march=native u8_sort.c -o u8_sort.so
cc -DBUILDING_u4_sort -D__BYTE_ORDER=__LITTLE_ENDIAN -DBUILDING_u4_sort -I/usr/include -I./ -I../ -I../../ -std=c99 -fgnu89-inline -O3 -g -fPIC -shared -march=native u4_sort.c -o u4_sort.so
popd
cp usort/usort/u8_sort.so .
cp usort/usort/u4_sort.so .

# clean
# rm -f parallelSort.cpp *.o *.so
