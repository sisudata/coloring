include ../defs.mk

APPS=u1 s1 u2 s2 f4 f8 u4 s4 u8 s8
SRC=$(patsubst %,%.c,$(wildcard $(APPS)))

all : $(APPS)
	echo $(SRC)

u1  : hsort-cmp.c  ../hsort.c $(SRC)  
	${CC} $(G) $(W) $(I) $(O) -o u1 u1.c 

s1  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s1 s1.c 

u2  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o u2 u2.c 

s2  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s2 s2.c 

f4  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o f4 f4.c 

s4  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s4 s4.c 

u4  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o u4 u4.c 

f8  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o f8 f8.c 

u8  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o u8 u8.c 

s8  : hsort-cmp.c  ../hsort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s8 s8.c 

clean :
	rm -Rf $(APPS) *~ *.dSYM
