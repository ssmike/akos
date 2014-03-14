RPATH=$(PWD)/../task-1
LDFLAGS=-L$(RPATH) -Wl,-rpath=$(RPATH) -lgetss
FLAGS= -pedantic -ansi -g -I$(RPATH)

main: clean
	gcc ./main.c -o ./main $(FLAGS) $(LDFLAGS)
run:
	./main
clean:
	rm -f ./main
