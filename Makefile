LDFLAGS=

all:
	mpicc sistRecom.c -lm -o sisre
	
compSec:
	mpicc sistRecomSec.c -lm -o sisre
	
run:
	mpirun -np 4 sisre

clean:
	rm -f *.o *.C~ Makefile~ sisre

