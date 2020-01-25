#!/usr/bin/zsh -e

echo "TESTING APP: " ${app:="f8"}
APP=$app

i=(8 128  1024 10000 100000 1000000 10000000)
t=(10000000 1000000 100000 10000 1000 100 10 3)

sz=${#i[@]}
c=1
echo "N               usort (secs)    GLIBC qsort (secs)    x-fold speedup"
c=1

while [ $c -le $sz ] ; do
    ./${APP} ${i[$c]} RAND ${t[$c]}  
    c=$(( $c + 1 ))
done;
