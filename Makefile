all: lib examples

CC = gcc
OBJ = lib/core.o lib/json.o lib/mdns.o lib/http.o
DEPS = lib/core.h lib/include/webthing.h

%.o: %.c $(DEPS)
	$(CC) -c -static $< -o $@ -Ilib/include -pthread

emhttp_lib.o: lib/emhttp_lib.c
	gcc -c -static lib/emhttp_lib.c -o emhttp_lib.o

lib: $(OBJ) emhttp_lib.o
	ar rcs webthing.a $(OBJ) emhttp_lib.o

examples: single_thing

single_thing: lib
	gcc -c examples/single_thing.c -o single_thing.o -Ilib/include
	gcc single_thing.o webthing.a lib/libcjson.a -o single_thing -pthread

clean:
	rm *.a *.o single_thing lib/*o -f
