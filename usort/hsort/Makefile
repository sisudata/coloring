include defs.mk

.PHONY: clean all cmp sample 

all:
	cd t/; make all

clean: 
	cd t/; make clean
	rm -f *~

cmp: all
	cd t; ./cmp-all.sh

