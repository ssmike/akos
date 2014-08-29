INCLUDE+=-I$(CURDIR)/include
ARCH=$(shell arch)
CFLAGS+=-g -pedantic -pthread -g $(INCLUDE)
LDFLAGS+=-lc -lpthread -e main -A $(ARCH)
CFILES=$(wildcard src/*.c)
OBJECTS=$(patsubst src/%,bin/%,$(CFILES:.c=.o))
LD=gcc

dist/main: bin dist $(OBJECTS)
	$(LD) $(OBJECTS) -o dist/main  $(LDFLAGS)

bin/%.o : src/%.c $(wildcard include/*.h)
	$(CC) -c $< -o $@ $(CFLAGS)

run: dist/main
	$(CURDIR)/dist/main

clean:
	rm -f *.o
	rm -rf bin dist
vimrc:
	rm -f ./.vim.local ./.mk
	@echo "let g:clang_user_options='$(INCLUDE)'" >> ./.vim.local
	@echo "let g:syntastic_c_compiler_options='$(CFLAGS)'" >> ./.vim.local
	@echo "all:" >> ./.mk
	@echo "	make -C .." >> ./.mk
	@echo "run:" >> ./.mk
	@echo "	make -C .. run" >> ./.mk
	cp .vim.local src
	cp .mk src/Makefile
	cp .vim.local include
	cp .mk include/Makefile
%:
	mkdir -p $@

.DEFAULT_GOAL: dist/main

.PHONY: clean
