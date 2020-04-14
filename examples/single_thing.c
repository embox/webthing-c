#include <stddef.h>

#include <webthing.h>

static void fade_action(struct webthing_action *self, void *arg) {
}

struct webthing *build_thing(void) {
	struct webthing *thing;
	thing = webthing_alloc();

	*thing = (struct webthing) {
		.name = "My lamp",
		.desc = "A web connected lamp",
	};

	struct webthing_property *brightness, *on_off, *duration;

	struct webthing_metadata on_off_meta = {
		.at_type = "OnOffProperty",
		.title = "On/Off",
		.type = "boolean",
		.description = "Whether the lamp is turned on",
	};

	on_off = webthing_property_create(&on_off_meta);
	webthing_add_property(thing, on_off);

	struct webthing_metadata brightness_meta = {
		.at_type     = "BrightnessProperty",
		.title       = "Brightness",
		.type        = "integer",
		.description = "The level of light from 0 to 100",
		.minimum     = 0,
		.maximum     = 100,
		.unit        = "percent",
	};

	brightness = webthing_property_create(&brightness_meta);
	webthing_add_property(thing, brightness);

	struct webthing_metadata duration_meta = {
		.type        = "integer",
		.minimum     = 1,
		.unit        = "seconds",
	};
	duration = webthing_property_create(&duration_meta);

	struct webthing_metadata action_meta = {
		.title = "Fade",
		.description = "Fade the lamp to a given level",
		.input = {
			.type = "object",
			.required = {
				"brightness",
				"duration",
			},
			.properties = {
				brightness,
				duration,
			},
		},
	};

	struct webthing_action *fade;
	fade = webthing_action_create(&action_meta, fade_action, NULL);

	webthing_add_action(thing, fade);

	return thing;
}

void destroy_thing(struct webthing *thing) {
	webthing_free(thing);
}

int main(int argc, char **argv) {
	struct webthing *thing;

	thing = build_thing();

	webthing_server_run(&thing, 1, "lamp", 8888);

	webthing_free(thing);

	return 0;
}
