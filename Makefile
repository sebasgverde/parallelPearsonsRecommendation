LDFLAGS=

all:
	mpicc mpi2.c -lm -o sr2

clean:
	rm -f *.o *.C~ Makefile~ sr2

