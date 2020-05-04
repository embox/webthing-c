/**
 * @file
 * @brief
 *
 * @author  Anton Kozlov
 * @date    15.04.2015
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "emhttp_lib.h"

static void httpd_parse_uri(char *str, struct http_req_uri *huri) {
	char *pb;
	pb = str;

	huri->target = pb;

	pb = strchr(pb, '?');
	if (pb) {
		*(pb++) = '\0';
		huri->query = pb;
	} else {
		pb = huri->target;
		huri->query = NULL;
	}
}

static char *httpd_parse_request_line(char *str, struct http_req *hreq) {
	char *pb;
	char *next_line;
	char *uri;

	next_line = strstr(str, "\r\n");
	if (!next_line) {
		printf("can't find sentinel\n");
		return NULL;
	}

	*next_line++ = '\0';
	*next_line++ = '\0';

	hreq->method = str;
	pb = strchr(str, ' ');
	if (!pb) {
		return NULL;
	}
	*(pb++) = '\0';

	uri = pb;
	pb = strchr(uri, ' ');
	if (!pb) {
		return NULL;
	}
	*pb = '\0';

	httpd_parse_uri(uri, &hreq->uri);

	return next_line;
}

char *http_parse_request(char *str, struct http_req *hreq) {
	char *next_line;

	hreq->fields = httpd_parse_request_line(str, hreq);
	if (!hreq->fields) {
		printf("can't parse request line\n");
		return NULL;
	}
	next_line = hreq->fields;

	while (memcmp(next_line, "\r\n", 2)) {
		next_line = strstr(next_line, "\r\n");
		if (!next_line) {
			printf("can't fine end of header\n");
			return NULL;
		}

		*next_line++ = '\0';
		*next_line++ = '\0';
	}
	return hreq->fields;
}

char *http_find_field(char *fields, char *field) {
	char full_field[0x40];
	int len;

	strncpy(full_field, field, sizeof(full_field) - 2 -1 );
	strcat(full_field, ": ");

	len = strlen(full_field);

	while(strncmp(fields, full_field, len)) {
		fields += (strlen(fields) + 2);
		if (0 == memcmp(fields, "\r\n", 2)) {
			return NULL;
		}
	}
	return fields + len;
}

char *http_get_text_response_header(void) {
	return "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"\r\n";
}

char *http_get_json_response_header(char *buf, int buf_size, int content_lengh) {
	snprintf(buf, buf_size, "HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n"
			"Access-Control-Allow-Methods: GET, HEAD, PUT, POST, DELETE\r\n"
			"Content-Length: %d\r\n"
			"Connection: close\r\n"
			"\r\n", content_lengh);
	return buf;
}

char *http_get_switching_response_header(char *buf, int buf_size, char *upgrade_type, char *sec_key) {
	snprintf(buf, buf_size, "HTTP/1.1 101 Switching protocols\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n"
			"Access-Control-Allow-Methods: GET, HEAD, PUT, POST, DELETE\r\n"
			"Upgrade: %s"
			"Connection: Upgrade\r\n"
			"Sec-Websocket-Accept: %s\r\n"
			"\r\n", upgrade_type, sec_key);
	return buf;
}
