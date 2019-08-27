#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <jansson.h>
#include <util/platform.h>

#include "aeffectx.h"

#include "../headers/vst-plugin-callbacks.hpp"

int mEffectID = 0;

static intptr_t
hostCallback_static(AEffect *effect, int32_t opcode, int32_t index, intptr_t value, void *ptr, float opt)
{
	switch (opcode) {
	case audioMasterVersion:
		return (intptr_t)2400;

	case audioMasterCanDo: {
		char *s = (char *)ptr;
		if (strcmp(s, "acceptIOChanges") == 0 || strcmp(s, "sendVstTimeInfo") == 0 ||
		    strcmp(s, "startStopProcess") == 0 || strcmp(s, "shellCategory") == 0 ||
		    strcmp(s, "sizeWindow") == 0) {
			return 1;
		}
		return 0;
	}

	case audioMasterCurrentId: {
		return mEffectID;
	}

	default:
		return 0;
	}
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv_w[])
#else
int main(int argc, char *argv[])
#endif
{
	if (argc != 2) {
		std::cerr << "Invalid number of arguments\n";
		return 1;
	}

		AEffect *plugin = nullptr;

	const char *path;

#ifdef _WIN32
	char name_utf8[MAX_PATH];
	os_wcs_to_utf8(argv_w[1], 0, name_utf8, MAX_PATH);
	path = name_utf8;
#else
	path = argv[1];
#endif


	auto          module         = os_dlopen(path);
	vstPluginMain mainEntryPoint = (vstPluginMain)os_dlsym(module, "VSTPluginMain");

	if (mainEntryPoint == nullptr) {
		mainEntryPoint = (vstPluginMain)os_dlsym(module, "VstPluginMain()");
	}

	if (mainEntryPoint == nullptr) {
		mainEntryPoint = (vstPluginMain)os_dlsym(module, "main");
	}

	if (mainEntryPoint == nullptr) {
		std::cerr << "Couldn't get a pointer to plug-in's main()";
		return 1;
	}

	// Instantiate the plug-in
	plugin = mainEntryPoint(hostCallback_static);

	auto jsonMain = json_object();

	json_object_set(jsonMain, "plugin_path", json_string(path));

	auto effectsJsonArray = json_array();

	// Are we a shell?
	if ((VstPlugCategory)plugin->dispatcher(plugin, effGetPlugCategory, 0, 0, NULL, 0.0f) == kPlugCategShell) {

		std::vector<int> effectIDs;

		char name[64];
		int  effectID;

		effectID = (int)plugin->dispatcher(plugin, effShellGetNextPlugin, 0, 0, &name, 0.0f);
		effectIDs.push_back(effectID);

		while (effectID) {
			effectID = (int)plugin->dispatcher(plugin, effShellGetNextPlugin, 0, 0, &name, 0.0f);
			effectIDs.push_back(effectID);
		}

		for (auto effect : effectIDs) {
			if (effect == 0) {
				continue;
			}

			mEffectID = effect;

			AEffect *subPlugin = mainEntryPoint(hostCallback_static);

			char buf[256];

			memset(buf, 0, sizeof(buf));
			subPlugin->dispatcher(subPlugin, effGetProductString, 0, 0, buf, 0);

			subPlugin->dispatcher(subPlugin, effClose, 0, 0, NULL, 0);

			auto effectJson = json_object();

			json_object_set(effectJson, "id", json_integer(effect));
			json_object_set(effectJson, "name", json_string(buf));

			json_array_append(effectsJsonArray, effectJson);
		}

	} else {
		char buf[256];

		memset(buf, 0, sizeof(buf));
		plugin->dispatcher(plugin, effGetProductString, 0, 0, buf, 0);

		auto effectJson = json_object();

		json_object_set(effectJson, "id", json_integer(0));
		json_object_set(effectJson, "name", json_string(buf));

		json_array_append(effectsJsonArray, effectJson);
	}

	json_object_set(jsonMain, "effects", effectsJsonArray);
	plugin->dispatcher(plugin, effClose, 0, 0, NULL, 0);

	os_dlclose(module);

	auto jsonString = json_dumps(jsonMain, NULL);

	std::cout << jsonString << "\n";

	free(jsonString);

	return 0;
}
