#include <stdio.h>

#include <cjson/cJSON.h>

#include <webthing.h>

char *json_thing(struct webthing *thing) {
	char *string;
	/* Webthing identification json string should look like this:
	{
		"id": "urn:dev:ops:my-lamp-1234",
		"title": "My Lamp",
		"@context": "https://iot.mozilla.org/schemas",
		"properties": {
			"on": {%
				"@type": "OnOffProperty",
				"title": "On/Off",
				"type": "boolean",
				"description": "Whether the lamp is turned on",
				"links": [{"rel": "property", "href": "/properties/on"}]
			},
		"brightness": {
			"@type": "BrightnessProperty",
			"title": "Brightness",
			"type": "integer",
			"description": "The level of light from 0-100",
			"minimum": 0,
			"maximum": 100,
			"unit": "percent",
			"links": [{"rel": "property", "href": "/properties/brightness"}]
			}
		},
		"actions": {
			"fade": {"title": "Fade",
			"description": "Fade the lamp to a given level",
			"input": {"type": "object",
			"required": ["brightness", "duration"],
			"properties": {"brightness": {"type": "integer",
			"minimum": 0,
			"maximum": 100,
			"unit": "percent"},
			"duration": {"type": "integer",
			"minimum": 1,
			"unit": "meters"}}},
			"links": [{"rel": "action",
			"href": "/actions/fade"}]}
		},
		"events": {"overheated": {"description": "The lamp has exceeded its safe operating temperature",
				"type": "number",
				"unit": "degree celsius",
				"links": [{"rel": "event",
				"href": "/events/overheated"}]}},
		"links": [{"rel": "properties",
			"href": "/properties"},
			{"rel": "actions", "href": "/actions"},
			{"rel": "events", "href": "/events"},
			{"rel": "alternate", "href": "ws://localhost:8888/"}],
		"description": "A web connected lamp",
		"@type": ["OnOffSwitch", "Light"],
		"base": "http://localhost:8888/",
		"securityDefinitions": {"nosec_sc": {"scheme": "nosec"}},
		"security": "nosec_sc"
	} */
	cJSON *jthing = cJSON_CreateObject();

	if (cJSON_AddStringToObject(jthing, "id", thing->id) == NULL) {
		goto out;
	}

	if (cJSON_AddStringToObject(jthing, "title", thing->name) == NULL) {
		goto out;
	}

	if (cJSON_AddStringToObject(jthing, "@context", "https://iot.mozilla.org/schemas") == NULL) {
		goto out;
	}

	if (cJSON_AddStringToObject(jthing, "description", thing->desc) == NULL) {
		goto out;
	}

	if (cJSON_AddStringToObject(jthing, "security", "nosec_sc") == NULL) {
		goto out;
	}

	string = cJSON_Print(jthing);
out:
	cJSON_Delete(jthing);

	return string;
}
