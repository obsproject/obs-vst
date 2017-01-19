#include "../headers/VSTPlugin.h"
#include "../headers/vst-plugin-callbacks.hpp"

#include <util/platform.h>
#include <windows.h>

AEffect* VSTPlugin::loadEffect() {
	AEffect *plugin = nullptr;

	wchar_t *wpath;
	os_utf8_to_wcs_ptr(pluginPath.c_str(), 0, &wpath);
	dllHandle = LoadLibraryW(wpath);
	bfree(wpath);
	if(dllHandle == nullptr) {
		printf("Failed trying to load VST from '%s', error %d\n",
			pluginPath, GetLastError());
		return nullptr;
	}

	vstPluginMain mainEntryPoint =
		(vstPluginMain)GetProcAddress(dllHandle, "VSTPluginMain");

	if (!mainEntryPoint) {
		return nullptr;
	}

	// Instantiate the plug-in
	plugin = mainEntryPoint(hostCallback_static);
	plugin->user = this;
	return plugin;
}

void VSTPlugin::unloadLibrary() {
	if (dllHandle) {
		FreeLibrary(dllHandle);
		dllHandle = nullptr;
	}
}