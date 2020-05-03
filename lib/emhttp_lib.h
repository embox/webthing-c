/*
 * emhttp_lib.h
 *
 *  Created on: Apr 15, 2020
 *      Author: anton
 */

#ifndef EMHTTP_LIB_H_
#define EMHTTP_LIB_H_

struct http_req_uri {
	char *target;
	char *query;
};

struct http_req {
	struct http_req_uri uri;
	char *method;
	char *fields;
};

extern char *http_parse_request(char *str, struct http_req *hreq);
extern char *http_get_text_response_header(void);
extern char *http_get_json_response_header(char *buf, int buf_size, int content_lengh);
extern char *http_find_field(char *fields, char *field);
extern char *http_get_switching_response_header(char *buf, int buf_size, char *upgrade_type);

#endif /* EMHTTP_LIB_H_ */
