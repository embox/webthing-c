#ifndef CORE_H
#define CORE_H

#include <stdint.h>

struct webthing;

struct mdns_args {
	const char *hostname;
	int port;
};

struct http_args {
	uint16_t port;
	struct webthing *thing;
};

char *json_thing(struct webthing *thing);

#endif /* CORE_H */
