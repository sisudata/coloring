I=-I/usr/include -I./ -I../ -I../../ -I../../../ -I../../../../
F=-std=c99 -fgnu89-inline
W=-Wall
O=-O3
G=-g
L=-lm
LD_LIBRARY_PATH=/usr/lib:/lib:
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

%.o : %.cc
	$(CC) ${F} $(I) $(W) $(O) -o $@ -c $<


#notes: c89 has no isfinite.  c99 inline is not supported.
