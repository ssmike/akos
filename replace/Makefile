RPATH=$(PWD)/../task-1
LDFLAGS=-L$(RPATH) -Wl,-rpath=$(RPATH) -lgetss
FLAGS= -pedantic -ansi -g -I$(RPATH)

main: deps clean
	gcc ./main.c -o ./main $(FLAGS) $(LDFLAGS)
run:
	./main
clean:
	rm -f ./main
deps:
	bash ./compile-deps.sh
