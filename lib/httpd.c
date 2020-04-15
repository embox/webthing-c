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

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdint.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "emhttp_lib.h"


#define BUFF_SZ   (1024)

struct client_info {
	struct sockaddr ci_addr;
	socklen_t ci_addrlen;
	int ci_sock;
	int ci_index;

	const char *ci_basedir;
};

static char httpd_g_inbuf[BUFF_SZ];


static int httpd_read_http_header(int sk, char *buf, size_t buf_sz) {
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

int httpd_build_request(struct client_info *cinfo, struct http_req *hreq, char *buf, size_t buf_sz) {
	int nbyte;

	nbyte = httpd_read_http_header(cinfo->ci_sock, buf, buf_sz - 1);
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

static int httpd_header(const struct client_info *cinfo, const char *msg, int msg_size) {
	int cbyte;
	char *buf;

	buf = http_get_text_response_header();

	cbyte = strlen(buf);

	if (0 > write(cinfo->ci_sock, buf, cbyte)) {
		return -errno;
	}

	if (0 > write(cinfo->ci_sock, msg, msg_size)) {
		return -errno;
	}

	return 0;
}

static void httpd_client_process(struct client_info *cinfo) {
	struct http_req hreq;
	int err;

	if (0 > (err = httpd_build_request(cinfo, &hreq, httpd_g_inbuf, sizeof(httpd_g_inbuf)))) {
		fprintf(stderr, "can't build request: %s\n", strerror(-err));
	}

	fprintf(stderr, "method=%s uri_target=%s uri_query=%s\n",
			   hreq.method, hreq.uri.target, hreq.uri.query);


	httpd_header(cinfo, "!!!!!!", sizeof("!!!!!!"));
}

struct httpd_args {
	uint16_t port;
};

void *httpd_thread(void *arg) {
	int host;
	struct sockaddr_in inaddr;
	const int family = AF_INET;
	struct httpd_args *a = arg;

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
		if (ci.ci_sock == -1) {
			if (errno != EINTR) {
				fprintf(stderr, "accept() failure: %s\n", strerror(errno));
				usleep(100000);
			}
			continue;
		}

		httpd_client_process(&ci);

		close(ci.ci_sock);
	}

	close(host);

	return NULL;
}
