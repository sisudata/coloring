#!/usr/bin/zsh -e

echo "TESTING APP: " ${app:="f8"}
APP=$app

i=(4 8 16 32 64 128 256 1024)
t=(1000000 1000000 1000000 100000 100000 20000 10000 5000 2000)
sz=${#i[@]}
c=1
echo "Sort Comparison of unsigned integers on N numbers generating according to:"
echo "RANDOM:    random integers"
echo "BOUNDED:   random() % (N/4)"
echo "SORTED:    sorted integers 1...N"
echo "REVERSED:  sorted integers N...1"
echo "IDENT:     all values of the array are \"1\""

echo "\nRANDOM"
echo "N               mysort (secs)   GLIBC (secs)    x-fold speedup"
c=1
while [ $c -lt $sz ] ; do
    ./${APP} ${i[$c]} RAND ${t[$c]}  
    c=$(( $c + 1 ))
done;
exit 0
if [ ! $app = "f4" -a ! $app = "f8" ] ; then
    echo "BOUNDED"
    c=1
    while [ $c -lt $sz ] ; do
        ./${APP} ${i[$c]} BOUNDED ${t[$c]}  
        c=$(( $c + 1 ))
    done;
fi;
echo "SORTED"
c=1
while [ $c -lt $sz ] ; do
    ./${APP} ${i[$c]} SORTED ${t[$c]}  
    c=$(( $c + 1 ))
done;
echo "REVERSED"
c=1
while [ $c -lt $sz ] ; do
    ./${APP} ${i[$c]} REVERSE ${t[$c]}  
    c=$(( $c + 1 ))
done;
echo "IDENT"
c=1
while [ $c -lt $sz ] ; do
    ./${APP} ${i[$c]} IDENT ${t[$c]}  
    c=$(( $c + 1 ))
done;
