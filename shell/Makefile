INCLUDE+=-I$(PWD)/$(INC_R_DIR)
CFLAGS+=-g -ansi -pedantic -Werror -Wextra $(INCLUDE)
LDFLAGS+=
CFILES=$(wildcard src/*)
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
	rm -f ./.vim.local
	@echo "let g:clang_user_options='$(INCLUDE)'" >> ./.vim.local
	@echo "let g:syntastic_c_compiler_options='-ansi -pedantic -Wall $(INCLUDE)'" >> ./.vim.local
	@echo "so ~/.vimrc" >> ./.vim.local
	cp .vim.local src
	cp .vim.local include
%:
	mkdir -p $@

