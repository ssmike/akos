SSP=$(PWD)/../task-1
LDFLAGS=-L$(SSP) -Wl,-rpath=$(SSP) -lgetss
INCLUDE=-I$(SSP) 
FLAGS= -pedantic -ansi -g $(INCLUDE)
CC=gcc


main: clean 
	$(CC) main.c -o main $(LDFLAGS) $(FLAGS)

run: main
	./main -5

deps:
	bash ./compile-deps.sh 2>/dev/null

clean:
	rm -f *.so
	rm -f *.o
	rm -f *.a
	rm -f main
