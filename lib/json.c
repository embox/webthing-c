#include <stdio.h>

#include <cjson/cJSON.h>

#include <webthing.h>

char *json_thing(struct webthing *thing) {
	char *ret;
	/* Missing fields from original example:
	{
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

	{
		cJSON *props = cJSON_CreateObject();
		int i = 0;

		while (thing->properties[i] != NULL) {
			struct webthing_property *p = thing->properties[i];
			cJSON *prop = cJSON_CreateObject();

			if (cJSON_AddStringToObject(prop, "@type", p->attype) == NULL) {
				goto out;
			}

			if (cJSON_AddStringToObject(prop, "title", p->title) == NULL) {
				goto out;
			}

			if (cJSON_AddStringToObject(prop, "type", p->type) == NULL) {
				goto out;
			}

			if (cJSON_AddStringToObject(prop, "description", p->description) == NULL) {
				goto out;
			}

			cJSON *link;
			if ((link = cJSON_AddArrayToObject(prop, "links")) == NULL) {
				printf("fail %d\n", __LINE__);
				goto out;
			}

			cJSON *link_prop = cJSON_CreateObject();
			char prop_str[1024];
			snprintf(prop_str, sizeof(prop_str), "/properties/%s", p->href);

			if ((cJSON_AddStringToObject(link_prop, "rel", "properties") == NULL) ||
					(cJSON_AddStringToObject(link_prop, "href", prop_str) == NULL)) {
				printf("fail %d\n", __LINE__);
				goto out;
			}

				printf("fail %d\n", __LINE__);
			cJSON_AddItemToObject(prop, p->href, link_prop);

			cJSON_AddItemToObject(props, p->href, prop);
			i++;
		}

		cJSON_AddItemToObject(jthing, "properties", cJSON_CreateObject());
	}
	cJSON_AddItemToObject(jthing, "brightness", cJSON_CreateObject());
	cJSON_AddItemToObject(jthing, "actions", cJSON_CreateObject());

	{
		cJSON *links;
		if ((links = cJSON_AddArrayToObject(jthing, "links")) == NULL) {
			goto out;
		}

		cJSON *link_prop = cJSON_CreateObject();
		if ((cJSON_AddStringToObject(link_prop, "rel", "properties") == NULL) ||
			(cJSON_AddStringToObject(link_prop, "href", "/properties") == NULL)) {
			goto out;
		}

		cJSON_AddItemToArray(links, link_prop);

		cJSON *actions_prop = cJSON_CreateObject();
		if ((cJSON_AddStringToObject(actions_prop, "rel", "actions") == NULL) ||
			(cJSON_AddStringToObject(actions_prop, "href", "/actions") == NULL)) {
			goto out;
		}

		cJSON_AddItemToArray(links, actions_prop);

		cJSON *events_prop = cJSON_CreateObject();
		if ((cJSON_AddStringToObject(events_prop, "rel", "events") == NULL) ||
			(cJSON_AddStringToObject(events_prop, "href", "/events") == NULL)) {
			goto out;
		}

		cJSON_AddItemToArray(links, events_prop);

		char wshost[1024];
		snprintf(wshost, sizeof(wshost), "ws://%s:%d", thing->hostname, thing->port);

		cJSON *alternate_prop = cJSON_CreateObject();
		if ((cJSON_AddStringToObject(alternate_prop, "rel", "alternate") == NULL) ||
			(cJSON_AddStringToObject(alternate_prop, "href", wshost) == NULL)) {
			goto out;
		}

		cJSON_AddItemToArray(links, alternate_prop);
	}

	if (cJSON_AddStringToObject(jthing, "description", thing->desc) == NULL) {
		goto out;
	}

	{
		/* TODO update types */
		const char *types[] = {
			"OnOffSwitch",
			"Light"
		};

		cJSON *strarr = cJSON_CreateStringArray(types, 2);

		cJSON_AddItemToObject(jthing, "@type", strarr);
	}

	{
		char httpcost[1024];
		snprintf(httpcost, sizeof(httpcost), "http://%s:%d", thing->hostname, thing->port);
		if (cJSON_AddStringToObject(jthing, "base", httpcost) == NULL) {
			goto out;
		}
	}

	{
		cJSON *inner = cJSON_CreateObject();
		cJSON *outer = cJSON_CreateObject();

		if (cJSON_AddStringToObject(inner, "scheme", "nosec") == NULL) {
			goto out;
		}

		cJSON_AddItemToObject(outer, "nosec_sc", inner);

		cJSON_AddItemToObject(jthing, "securityDefinitions", outer);

		if (cJSON_AddStringToObject(jthing, "security", "nosec_sc") == NULL) {
			goto out;
		}
	}

	ret = cJSON_Print(jthing);

	printf("%s\n", ret);
out:
	cJSON_Delete(jthing);

	return ret;
}
