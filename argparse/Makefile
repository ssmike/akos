SSP=$(PWD)/../task-1
ARGS=$(PWD)/../argparse
LDFLAGS=-L$(SSP) -L$(ARGS) -Wl,-rpath=$(SSP),-rpath=$(ARGS) -lgetss
INCLUDE=-I$(SSP) -I$(ARGS)
FLAGS= -pedantic -ansi -g $(INCLUDE)
CC=gcc


main: clean libs
	$(CC) client.c -o main $(LDFLAGS) $(FLAGS)

run:
	./main

libs: deps
	$(CC) -c -fPIC argparse.c $(FLAGS)
	$(CC) -shared argparse.o -o libargparse.so
	ar -rsc libargparse.a argparse.o

deps:
	bash ./compile-deps.sh 2>/dev/null

clean:
	rm -f *.so
	rm -f *.o
	rm -f *.a
	rm -f main
