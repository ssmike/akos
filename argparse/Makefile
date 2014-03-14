SSP=$(PWD)/../task-1
ARGS=$(PWD)/../argparse
LDFLAGS=-L$(SSP) -L$(ARGS) -Wl,-rpath=$(SSP),-rpath=$(ARGS) -lgetss
FLAGS= -pedantic -ansi -g -I$(SSP) -I$(ARGS)
CC=gcc


main: clean libs
	$(CC) client.c -o main $(LDFLAGS) $(FLAGS)

libs:
	$(CC) -c -fPIC argparse.c $(CFLAGS)
	$(CC) -shared argparse.o -o libargparse.so
	ar -rsc libargparse.a argparse.o

clean:
	rm -f *.so
	rm -f *.o
	rm -f *.a
	rm -f main
