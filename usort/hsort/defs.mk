I=-I/usr/include -I../ -I../../ -I../../../
W=-Wall
O=-O3 -g
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

%.o : %.c
	$(CC) $(I) $(W) $(O) -o $@ -c $<

