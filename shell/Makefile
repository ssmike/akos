INCLUDE+=-I$(PWD)/include
CFLAGS+=-g -ansi -pedantic -Werror -Wextra $(INCLUDE)
LDFLAGS+=
CFILES=$(wildcard src/*.c)
OBJECTS=$(patsubst src/%,bin/%,$(CFILES:.c=.o))
CC=gcc

dist/main: bin dist $(OBJECTS)
	$(CC) $(OBJECTS) -o dist/main  $(LDFLAGS)

bin/%.o : src/%.c $(wildcard include/*.h)
	$(CC) -c $< -o $@ $(CFLAGS)

run: dist/main
	./dist/main

clean:
	rm -f *.o
	rm -rf ./bin ./dist
vimrc:
	rm -f ./.vim.local ./.mk
	@echo "let g:clang_user_options='$(INCLUDE)'" >> ./.vim.local
	@echo "let g:syntastic_c_compiler_options='-ansi -pedantic -Wall $(INCLUDE)'" >> ./.vim.local
	@echo "all:" >> ./.mk
	@echo "	make -C .." >> ./.mk
	cp .vim.local src
	cp .mk src/Makefile
	cp .vim.local include
	cp .mk include/Makefile
%:
	mkdir -p $@

