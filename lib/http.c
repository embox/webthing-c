/**
 * @file
 * @brief Simple HTTP server
 * @date 16.04.12
 * @author Ilia Vaprol
 * @author Anton Kozlov
 * 	- CGI related changes
 * @author Andrey Golikov
 * 	- Linux adaptation
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

 #include <openssl/sha.h>

#include "core.h"
#include "emhttp_lib.h"
#include "b64.h"

#include <webthing.h>

#define BUFF_SZ   (1024)

struct client_info {
	struct sockaddr ci_addr;
	socklen_t ci_addrlen;
	int ci_sock;
	int ci_index;

	const char *ci_basedir;

	struct webthing *thing;
};

static char http_g_inbuf[BUFF_SZ];

static int http_read_http_header(int sk, char *buf, size_t buf_sz) {
	const char pattern[] = "\r\n\r\n";
	char *pb;
	int offset;
	int header_len;

	if (buf_sz <  sizeof(pattern)) {
		return -EINVAL;
	}

	if (0 > read(sk, buf, 4)) {
		return -errno;
	}

	pb = buf + 4;

	for (header_len = 4; header_len < buf_sz; header_len++) {
		if (0 == memcmp(pb - 4, pattern, 4)) {
			break;
		}
		if (0 > read(sk, pb, 1)) {
			return -errno;
		}
		pb++;
	}

	*pb = '\0';

	return header_len;
}

int http_build_request(struct client_info *cinfo, struct http_req *hreq, char *buf, size_t buf_sz) {
	int nbyte;

	nbyte = http_read_http_header(cinfo->ci_sock, buf, buf_sz - 1);
	if (nbyte < 0) {
		fprintf(stderr, "can't read from client socket: %s\n", strerror(errno));
		return -errno;
	}

	memset(hreq, 0, sizeof(*hreq));
	if (NULL == http_parse_request(buf, hreq)) {
		fprintf(stderr, "can't parse request\n");
		return -EINVAL;
	}

	return nbyte;
}

static int http_header(const struct client_info *cinfo, const char *msg, int msg_size) {
	int cbyte;
	char *buf;
	char header_buf[4000];

	buf = http_get_json_response_header(header_buf, sizeof(header_buf), msg_size);

	cbyte = strlen(buf);

	if (0 > write(cinfo->ci_sock, buf, cbyte)) {
		return -errno;
	}

	if (0 > write(cinfo->ci_sock, msg, msg_size)) {
		return -errno;
	}

	return 0;
}

static void http_client_process(struct client_info *cinfo) {
	struct http_req hreq;
	int err;
	char *resp;
	char *connection;

	if (0 > (err = http_build_request(cinfo, &hreq, http_g_inbuf, sizeof(http_g_inbuf)))) {
		fprintf(stderr, "can't build request: %s\n", strerror(-err));
	}

	connection = http_find_field(hreq.fields, "Connection");

	fprintf(stderr, "method=%s uri_target=%s uri_query=%s connection=%s\n",
			   hreq.method, hreq.uri.target, hreq.uri.query, connection);
	if (NULL == connection) {
		printf("didn't find Connection field\n");
	} else if (0 == strcmp(connection,"close")) {
		resp = json_thing(cinfo->thing);

		printf("rest=%s\n", resp);

		http_header(cinfo, resp, strlen(resp));
	} else {
		char header_buf[400];
		int cbyte;
		char *key;
		char sec_key[100];

		key = http_find_field(hreq.fields, "Sec-WebSocket-Key");
		strncpy(sec_key, key, sizeof(sec_key)-1);
		sec_key[sizeof(sec_key)-1] = '\0';
		strncat(sec_key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", sizeof(sec_key)-1);
		sec_key[sizeof(sec_key)-1] = '\0';


		size_t length = strlen(sec_key);

		unsigned char hash[200];
		char out_key[32];
		size_t out_coded_sz;
		SHA1(sec_key, length, hash);
		b64_encode(hash, 20, out_key, 32, &out_coded_sz);

		resp = http_get_switching_response_header(header_buf, sizeof(header_buf), "websocket", out_key);

		cbyte = strlen(resp);

		if (0 > write(cinfo->ci_sock, resp, cbyte)) {
			return;
		}

		printf("switch connection\n");

	}
}

void *http_thread(void *arg) {
	int host;
	struct sockaddr_in inaddr;
	const int family = AF_INET;
	struct http_args *a = arg;

	inaddr.sin_family = AF_INET;
	inaddr.sin_port= htons(a->port);
	inaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	host = socket(family, SOCK_STREAM, IPPROTO_TCP);
	if (host == -1) {
		fprintf(stderr, "socket() failure: %s\n", strerror(errno));
		return (void *)(uintptr_t)-errno;
	}

	if (-1 == bind(host, (struct sockaddr *) &inaddr, sizeof(inaddr))) {
		fprintf(stderr, "bind() failure: %s\n", strerror(errno));
		close(host);
		return (void *)(uintptr_t)-errno;
	}

	if (-1 == listen(host, 1)) {
		fprintf(stderr, "listen() failure: %s\n", strerror(errno));
		close(host);
		return (void *)(uintptr_t)-errno;
	}

	while (1) {
		struct client_info ci;

		ci.ci_addrlen = sizeof(inaddr);
		ci.ci_sock = accept(host, &ci.ci_addr, &ci.ci_addrlen);
		ci.thing = a->thing;
		if (ci.ci_sock == -1) {
			if (errno != EINTR) {
				fprintf(stderr, "accept() failure: %s\n", strerror(errno));
				usleep(100000);
			}
			continue;
		}

		http_client_process(&ci);

		close(ci.ci_sock);
	}

	close(host);

	return NULL;
}
