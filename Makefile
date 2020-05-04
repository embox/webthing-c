all: lib examples

CC = gcc
OBJ = lib/core.o lib/json.o lib/mdns.o lib/http.o lib/b64.o
DEPS = lib/core.h lib/include/webthing.h

%.o: %.c $(DEPS)
	$(CC) -g -c -static $<  -Ilib/include  -o $@ -pthread -lssl -lcrypto

emhttp_lib.o: lib/emhttp_lib.c
	gcc -g -c -static lib/emhttp_lib.c -o emhttp_lib.o

lib: $(OBJ) emhttp_lib.o
	ar rcs webthing.a $(OBJ) emhttp_lib.o

examples: single_thing

single_thing: lib
	gcc -g -c examples/single_thing.c -o single_thing.o -Ilib/include
	gcc single_thing.o webthing.a lib/libcjson.a -o single_thing -pthread -lssl -lcrypto 

clean:
	rm *.a *.o single_thing lib/*o -f
