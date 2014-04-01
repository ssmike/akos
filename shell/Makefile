CFLAGS+=-g -ansi -pedantic -Werror -Wextra
LDFLAGS+=
ALL_NEEDED=shell_structs.h
CFILES=$(wildcard *.c)

OBJECTS=$(CFILES:.c=.o)
CC=gcc

main: $(OBJECTS)
	$(CC) $(OBJECTS) -o main  $(LDFLAGS)

%.o : %.c %.h $(ALL_NEEDED)
	$(CC) -c $< -o $@ $(CFLAGS)

run: main
	./main

clean:
	rm -f *.o
	rm -f main
