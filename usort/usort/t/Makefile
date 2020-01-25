include ../defs.mk

APPS=u1 u2 u4 s4 u8 s1 s2 s8 f4 f8
SRC=$(patsubst %,%.c,$(wildcard $(APPS)))

all : $(APPS)
	echo $(SRC)

u1  : ctype-cmp.c  $(SRC)  
	$(CC) -o u1 u1.c ${F} $(G) $(W) $(I) $(O) $(L)

s1  : ctype-cmp.c  $(SRC)  
	$(CC) -o s1 s1.c ${F} $(G) $(W) $(I) $(O) $(L)

s2  : ctype-cmp.c  $(SRC)  
	$(CC) -o s2 s2.c ${F} $(G) $(W) $(I) $(O) $(L)

u4  : ctype-cmp.c  $(SRC)  
	$(CC) -o u4 u4.c ${F} $(G) $(W) $(I) $(O) $(L)

s4  : ctype-cmp.c  $(SRC)  
	$(CC) -o s4 s4.c ${F} $(G) $(W) $(I) $(O) $(L)

u2  : ctype-cmp.c  $(SRC)  
	$(CC) -o u2 u2.c ${F} $(G) $(W) $(I) $(O) $(L)

u8  : ctype-cmp.c  $(SRC)  
	$(CC) -o u8 u8.c ${F} $(G) $(W) $(I) $(O) $(L)

s8  : ctype-cmp.c  $(SRC)  
	$(CC) -o s8 s8.c ${F} $(G) $(W) $(I) $(O) $(L)

f4  : ctype-cmp.c $(SRC)
	$(CC) -o f4 f4.c ${F} $(G) $(W) $(I) $(O) $(L)

f8  : ctype-cmp.c $(SRC)
	$(CC) -o f8 f8.c ${F} $(G) $(W) $(I) $(O) $(L)


clean :
	rm -Rf $(APPS) *.dSYM *~
