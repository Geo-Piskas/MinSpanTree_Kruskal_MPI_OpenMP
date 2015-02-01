CC=mpicc
CFLAGS=-Wall -O3 -fopenmp
BIN=../../bin/
BUILD=../../build/

all: kruskal

kruskal: main.o
	$(CC) $(CFLAGS) -o $(BIN)kruskal-mpi $(BUILD)main.o

main.o: $(SRC)main.c $(SRC)disjointset.h $(SRC)edge.h $(SRC)qsort_omp.h
	$(CC) $(CFLAGS) -o $(BUILD)main.o -c $(SRC)main.c

