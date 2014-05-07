INCLUDE+=-I$(CURDIR)
CFLAGS+=-g -pedantic -pthread $(INCLUDE)
LDFLAGS+=-pthread
CFILES=$(wildcard *.c)
OBJECTS=$(CFILES:.c=.o)
CC=gcc

main: $(OBJECTS)
	$(CC) $(OBJECTS) -o main  $(LDFLAGS)

%.o : %.c $(wildcard *.h)
	$(CC) -c $< -o $@ $(CFLAGS)

run: main
	$(CURDIR)/main ./inp1 ./inp2

clean:
	rm -f *.o
vimrc:
	rm -f ./.vim.local ./.mk
	@echo "let g:clang_user_options='$(INCLUDE)'" >> ./.vim.local
	@echo "let g:syntastic_c_compiler_options='$(CFLAGS)'" >> ./.vim.local
	@echo "all:" >> ./.mk
	@echo "	make -C .." >> ./.mk
	@echo "run:" >> ./.mk
	@echo "	make -C .. run" >> ./.mk
%:
	mkdir -p $@

.PHONY: clean
