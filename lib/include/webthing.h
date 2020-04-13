#ifndef WEBTHING_H
#define WEBTHING_H

struct webthing;
struct webthing_property;
struct webthing_action;
struct webthing_metadata;

#define WEBTHING_MAX_PROPS   32
#define WEBTHING_MAX_INPUTS  32
#define WEBTHING_MAX_ACTIONS 32

struct webthing {
	const char *name;
	const char *desc;

	struct webthing_property *properties[WEBTHING_MAX_PROPS];
	struct webthing_action   *actions[WEBTHING_MAX_ACTIONS];
};

struct webthing_property {
};

struct webthing_input {
	const char *type;
	const char *required[WEBTHING_MAX_INPUTS];

	const struct webthing_property *properties[WEBTHING_MAX_PROPS];
};

struct webthing_metadata {
	const char *at_type;
	const char *title;
	const char *type;
	const char *description;

	int minimum;
	int maximum;

	const char *unit;

	struct webthing_input input;
};

struct webthing_action {
};

struct webthing *webthing_alloc(void);
void webthing_free(struct webthing *thing);

struct webthing_property *webthing_property_create(struct webthing_metadata *meta);
void webthing_property_destroy(struct webthing_property *prop);
int webthing_add_property(struct webthing *thing, struct webthing_property *prop);

struct webthing_action *webthing_action_create(struct webthing_metadata *meta,
		void (*handler)(struct webthing_action *self, void *arg), void *arg);
void webthing_action_destroy(struct webthing_action *action);
int webthing_add_action(struct webthing *thing, struct webthing_action *action);

int webthing_server_run(struct webthing **devs, int dev_n);
int webthing_run(struct webthing *thing);

#endif /* WEBTHING_H */
