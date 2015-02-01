CC=gcc
CFLAGS=-Wall -O3
BIN=../../bin/
BUILD=../../build/

all: kruskal

kruskal: main.o
	$(CC) $(CFLAGS) -o $(BIN)kruskal-ser $(BUILD)main.o

main.o: $(SRC)main.c $(SRC)disjointset.h $(SRC)edge.h
	$(CC) $(CFLAGS) -o $(BUILD)main.o -c $(SRC)main.c

