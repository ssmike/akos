client: clean libs
	gcc -L$(PWD) -lgetss client.c -o client -Wl,-rpath=$(PWD)

clean:
	rm -f ./*.o ./*.so ./client ./*.a

run: client
	./client

libs: objects
	gcc -shared getss.o -o libgetss.so
	ar -rsc libgetss.a getss.o

objects:
	gcc -c -fPIC getss.c
