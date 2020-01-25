#!/usr/bin/zsh -e

echo "TESTING APP: " ${app:="f8"}
APP=$app

i=(8 16 32 64 128 )
t=(10000 10000 10000 10000 1000)

sz=${#i[@]}
c=1
echo "N               usort (secs)    GLIBC qsort (secs)    x-fold speedup"
c=1

while [ $c -le $sz ] ; do
    ./${APP} ${i[$c]} RAND ${t[$c]}  
    c=$(( $c + 1 ))
done;
