LDFLAGS=

all:
	cc sistema_recomendacion_seq.c -lm -o sr

clean:
	rm -f *.o *.C~ Makefile~

