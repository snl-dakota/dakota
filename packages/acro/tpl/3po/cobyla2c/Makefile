# Makefile for Cobyla
# $Jeannot: Makefile,v 1.8 2004/04/13 16:02:01 js Exp $

all: example

example: cobyla.o example.o
	$(CC) $(CFLAGS) -o example cobyla.o example.o -lm

test: example
	./example

# Python targets
moduleCobyla.so: moduleCobyla.c cobyla.c
	python setup.py build_ext --inplace

test-py: moduleCobyla.so
	python cobyla.py

example-py: moduleCobyla.so
	python example.py

clean:
	rm -f example *.o *.so *.pyc
	rm -rf build
