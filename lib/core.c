/**
 * @file core.c
 * @brief
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version
 * @date 08.04.2020
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <netdb.h>
#include <webthing.h>

struct mdns_args {
	const char *hostname;
	int port;
};

struct httpd_args {
	uint16_t port;
};

struct httpd_args httpd_args;
static struct mdns_args a;

void *mdns_thread(void *arg);

struct client_info {
	struct sockaddr ci_addr;
	socklen_t ci_addrlen;
	int ci_sock;
	int ci_index;

	const char *ci_basedir;
};
char *json_thing(struct webthing *thing);

int webthing_server_run(struct webthing **devs, int dev_n, const char *hostname, int port) {
	pthread_t id;
	struct sockaddr_in inaddr;
	const int family = AF_INET;

	a.hostname = hostname;
	a.port = 5353;

	fprintf(stderr, "Starting webthing server %s:%d\n", a.hostname, a.port);

	pthread_create(&id, NULL, mdns_thread, &a);

	inaddr.sin_family = AF_INET;
	inaddr.sin_port= htons(8890);
	inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int host = socket(family, SOCK_STREAM, IPPROTO_TCP);
	if (host == -1) {
		printf("socket() failure: %s", strerror(errno));
		return -errno;
	}

	if (-1 == bind(host, (struct sockaddr *) &inaddr, sizeof(inaddr))) {
		printf("bind() failure: %s", strerror(errno));
		close(host);
		return -errno;
	}

	if (-1 == listen(host, 1)) {
		printf("listen() failure: %s", strerror(errno));
		close(host);
		return -errno;
	}

	while (1) {
		struct client_info ci;

		ci.ci_basedir = "./";
		ci.ci_addrlen = sizeof(inaddr);
		ci.ci_sock = accept(host, &ci.ci_addr, &ci.ci_addrlen);
		if (ci.ci_sock == -1) {
			if (errno != EINTR) {
				printf("accept() failure: %s", strerror(errno));
				usleep(100000);
			}
			continue;
		}

		char buf[1024];
		int buf_sz = sizeof(buf);
		char *str = json_thing(devs[0]);
		printf("Got some http data");
		int cbyte = snprintf(buf, 1024,
				"HTTP/1.1 200\r\n"
				"Content-Type: %s\r\n"
				"Connection: close\r\n"
				"Content-Length: %d\r\n"
				"\r\n",
				"application/json", strlen(str));


//		if (0 > write(ci.ci_sock, buf, cbyte)) {
//			return -errno;
//		}
//
		strcat(buf, str);
		printf("json is %s\n", str);
		if (0 > write(ci.ci_sock, buf, strlen(buf))) {
			return -errno;
		}

		sleep(100);
		//httpd_client_process(&ci);

		close(ci.ci_sock);
	}

	return 0;
}

int webthing_sever_stop(void) {
	return 0;
}

struct webthing_property *webthing_property_create(struct webthing_metadata *meta) {
	struct webthing_property *ret = malloc(sizeof(*ret));
	return ret;
}

void webthing_property_destroy(struct webthing_property *prop) {
	free(prop);
}

int webthing_add_property(struct webthing *thing, struct webthing_property *prop) {
	for (int i = 0; i < WEBTHING_MAX_PROPS; i++) {
		if (thing->properties[i] == NULL) {
			thing->properties[i] = prop;
			return 0;
		}
	}

	return -ENOMEM;
}

struct webthing_action *webthing_action_create(struct webthing_metadata *meta,
		void (*handler)(struct webthing_action *self, void *arg), void *arg) {
	struct webthing_action *ret = malloc(sizeof(*ret));
	return ret;
}

void webthing_action_destroy(struct webthing_action *action) {
	free(action);
}

int webthing_add_action(struct webthing *thing, struct webthing_action *action) {
	for (int i = 0; i < WEBTHING_MAX_ACTIONS; i++) {
		if (thing->actions[i] == NULL) {
			thing->actions[i] = action;
			return 0;
		}
	}

	return -ENOMEM;
}

struct webthing *webthing_alloc(void) {
	return malloc(sizeof(struct webthing));
}

void webthing_free(struct webthing *thing) {
	free(thing);
}
