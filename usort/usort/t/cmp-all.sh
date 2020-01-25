#!/bin/sh -e

apps="u1 s1 u2 s2 u4 s4 f4 u8 s8 f8"

echo "Univeral Sort Functions (usort or ufunc sorters) are fast sorting "
echo "algorithms specicialized for each of the basic C numeric types"

echo "Comparison of ufunc sorters against GLIBC qsort baseline"
echo "u1 - unsigned char"
echo "s1 - signed char"
echo "u2 - unsigned short"
echo "s2 - signed short"
echo "u4 - unsigned int"
echo "s4 - signed int"
echo "f4 - 4 byte float"
echo "u8 - unsigned long long"
echo "s8 - signed long long"
echo "f8 - 8 byte float (double)."

for app in $apps ; do
    export app
    ./cmp.sh
done

