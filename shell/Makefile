CFLAGS+=-g -ansi -pedantic -Werror -Wextra
LDFLAGS+=
CFILES=$(wildcard *.c)
OBJECTS=$(addprefix bin/,$(CFILES:.c=.o))
CC=gcc

dist/main: bin dist $(OBJECTS)
	$(CC) $(OBJECTS) -o dist/main  $(LDFLAGS)


bin/%.o : %.c $(wildcard *.h)
	$(CC) -c $< -o $@ $(CFLAGS)

run: dist/main
	./dist/main

clean:
	rm -f *.o
	rm -rf ./bin ./dist

%:
	mkdir -p $@

