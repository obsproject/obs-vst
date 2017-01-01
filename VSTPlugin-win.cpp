#include "VSTPlugin.h"
#include "vst-plugin-callbacks.hpp"

#include <util/platform.h>
#include <windows.h> 

VstIntPtr VSTCALLBACK
HostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
	VstIntPtr result = 0;

	// Filter idle calls...
	bool filtered = false;
	if (opcode == audioMasterIdle) {
		static bool wasIdle = false;
		if (wasIdle)
			filtered = true;
		else {
			printf("(Future idle calls will not be displayed!)\n");
			wasIdle = true;
		}
	}

	switch (opcode) {
	case audioMasterVersion:
		result = kVstVersion;
		break;
	}

	return result;
}

AEffect* VSTPlugin::loadEffect() {
	AEffect *plugin = NULL;

	wchar_t *wpath;
	os_utf8_to_wcs_ptr(pluginPath.c_str(), 0, &wpath);
	dllHandle = LoadLibraryW(wpath);
	bfree(wpath);
	if(dllHandle == NULL) {
		printf("Failed trying to load VST from '%s', error %d\n",
			pluginPath, GetLastError());
		return NULL;
	}

	vstPluginFuncPtr mainEntryPoint =
		(vstPluginFuncPtr)GetProcAddress(dllHandle, "VSTPluginMain");
	// Instantiate the plugin
	plugin = mainEntryPoint(HostCallback);
	return plugin;
}

void VSTPlugin::unloadEffect() {
	effectReady = false;

	if (effect) {
		effect->dispatcher(effect, effMainsChanged, 0, 0, 0, 0);
		effect->dispatcher(effect, effClose, 0, 0, NULL, 0.0f);
	}

	effect = NULL;

	if (dllHandle) {
		FreeLibrary(dllHandle);
		dllHandle = NULL;
	}
}