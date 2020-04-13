/**
 * @file core.c
 * @brief
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version
 * @date 08.04.2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <webthing.h>

int webthing_server_run(struct webthing **devs, int dev_n, const char *hostname, int port) {
	fprintf(stderr, "Starting webthing server %s:%d\n");
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
			thing->actions[i] = prop;
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
