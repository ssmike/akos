CFLAGS=-Wall -ansi -pedantic
CC=gcc

client: clean libs
	$(CC) -L$(PWD) -lgetss client.c -o client -Wl,-rpath=$(PWD) $(CFLAGS)

clean:
	rm -f ./*.o ./*.so ./client ./*.a

run: client
	./client

libs: objects
	$(CC) -shared getss.o -o libgetss.so
	ar -rsc libgetss.a getss.o

objects:
	$(CC) -c -fPIC getss.c $(CFLAGS)
