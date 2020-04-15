all: lib examples

core.o: lib/core.c 
	gcc -c -static lib/core.c -o core.o -Ilib/include -pthread

json.o: lib/json.c
	gcc -c -static lib/json.c -o json.o -Ilib/include

mdns.o: lib/mdns.c
	gcc -c -static lib/mdns.c -o mdns.o -pthread

lib: core.o mdns.o json.o
	ar rcs webthing.a core.o mdns.o json.o lib/libcjson.a

examples: single_thing

single_thing:
	gcc -c examples/single_thing.c -o single_thing.o -Ilib/include
	gcc single_thing.o webthing.a lib/libcjson.a -o single_thing -pthread

clean:
	rm *.a *.o single_thing
