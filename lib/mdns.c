#include "mdns.h"
#include "core.h"

#include <stdio.h>
#include <errno.h>

#include <netdb.h>

static char addrbuffer[64];
static char namebuffer[256];
static char sendbuffer[256];
static mdns_record_txt_t txtbuffer[128];

typedef struct {
	const char* service;
	const char* hostname;
	uint32_t address_ipv4;
	uint8_t* address_ipv6;
	int port;
} service_record_t;

static mdns_string_t
ipv4_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in* addr, size_t addrlen) {
	char host[NI_MAXHOST] = {0};
	char service[NI_MAXSERV] = {0};
	int ret = getnameinfo((const struct sockaddr*)addr, addrlen,
	                      host, NI_MAXHOST, service, NI_MAXSERV,
	                      NI_NUMERICSERV | NI_NUMERICHOST);
	int len = 0;
	if (ret == 0) {
		if (addr->sin_port != 0)
			len = snprintf(buffer, capacity, "%s:%s", host, service);
		else
			len = snprintf(buffer, capacity, "%s", host);
	}
	if (len >= (int)capacity)
		len = (int)capacity - 1;
	mdns_string_t str = {buffer, len};
	return str;
}

static mdns_string_t
ipv6_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in6* addr, size_t addrlen) {
	char host[NI_MAXHOST] = {0};
	char service[NI_MAXSERV] = {0};
	int ret = getnameinfo((const struct sockaddr*)addr, addrlen,
	                      host, NI_MAXHOST, service, NI_MAXSERV,
	                      NI_NUMERICSERV | NI_NUMERICHOST);
	int len = 0;
	if (ret == 0) {
		if (addr->sin6_port != 0)
			len = snprintf(buffer, capacity, "[%s]:%s", host, service);
		else
			len = snprintf(buffer, capacity, "%s", host);
	}
	if (len >= (int)capacity)
		len = (int)capacity - 1;
	mdns_string_t str = {buffer, len};
	return str;
}

static mdns_string_t
ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen) {
	if (addr->sa_family == AF_INET6)
		return ipv6_address_to_string(buffer, capacity, (const struct sockaddr_in6*)addr, addrlen);
	return ipv4_address_to_string(buffer, capacity, (const struct sockaddr_in*)addr, addrlen);
}

static int
service_callback(int sock, const struct sockaddr* from, size_t addrlen,
                mdns_entry_type_t entry, uint16_t transaction_id,
                uint16_t rtype, uint16_t rclass, uint32_t ttl,
                const void* data, size_t size, size_t offset, size_t length,
                void* user_data) {
	if (entry != MDNS_ENTRYTYPE_QUESTION)
		return 0;
	mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, sizeof(addrbuffer), from, addrlen);
	if (rtype == MDNS_RECORDTYPE_PTR) {
		mdns_string_t service = mdns_string_extract(data, size, &offset,
		                                            namebuffer, sizeof(namebuffer));
		printf("%.*s : question PTR %.*s\n",
		       MDNS_STRING_FORMAT(fromaddrstr), MDNS_STRING_FORMAT(service));

		const service_record_t* service_record = (const service_record_t*)user_data;
		size_t service_length = strlen(service_record->service);
		if ((service.length == service_length) && (strncmp(service.str, service_record->service, service_length) == 0)) {
			printf("  --> answer %s.%s port %d\n", service_record->hostname, service_record->service, service_record->port);
			mdns_query_answer(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
			                  transaction_id, service_record->service, service_length,
			                  service_record->hostname, strlen(service_record->hostname),
							  service_record->address_ipv4, service_record->address_ipv6,
			                  (uint16_t)service_record->port, 0, 0);
		}
	}
	return 0;
}

void *mdns_thread(void *arg) {
	size_t capacity = 2048;
	void* buffer = malloc(capacity);
	void* user_data = 0;
	size_t records;

	struct mdns_args *m = arg;
	int mode = 0;
	const char* service = "_webthing._tcp.local.";
	const char* hostname = m->hostname;

	int port = m->port;
	int sock = mdns_socket_open_ipv4(port);
	if (sock < 0) {
		printf("Failed to open socket: %s\n", strerror(errno));
		return NULL;
	}
	printf("Opened IPv4 socket for mDNS/DNS-SD\n");

	while (1) {
		const int flags = fcntl(sock, F_GETFL, 0);
		fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);

		uint32_t address_ipv4 = 0;
		uint8_t* address_ipv6 = 0;
		uint8_t address_ipv6_buffer[16];

		service_record_t service_record = {
			service,
			hostname,
			address_ipv4,
			address_ipv6,
			5353
		};

		int error_code = 0;
		do {
			mdns_socket_listen(sock, buffer, capacity, service_callback, &service_record);
			int error_code_size = sizeof(error_code);
			getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error_code, (socklen_t*)&error_code_size);
		} while (!error_code);
	}
quit:
	free(buffer);

	mdns_socket_close(sock);
	printf("Closed socket\n");

	return NULL;
}
