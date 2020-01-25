include ../defs.mk

APPS=u1 s1 u2 s2 f4 f8 u4 s4 u8 s8
SRC=$(patsubst %,%.c,$(wildcard $(APPS)))

all : $(APPS)

u1  : csort-cmp.c ../swap.c ../csort.c $(SRC)  
	${CC} $(G) $(W) $(I) $(O) -o u1 u1.c ${LIB}

s1  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s1 s1.c ${LIB} 

u2  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o u2 u2.c ${LIB}

s2  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s2 s2.c ${LIB}

f4  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o f4 f4.c ${LIB}

s4  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s4 s4.c ${LIB}

u4  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o u4 u4.c ${LIB}

f8  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o f8 f8.c ${LIB}

u8  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o u8 u8.c ${LIB}

s8  : csort-cmp.c ../swap.c ../csort.c  $(SRC)
	${CC} $(G) $(W) $(I) $(O) -o s8 s8.c ${LIB}


clean :
	rm -Rf $(APPS) csort-cmp *~ *.dSYM