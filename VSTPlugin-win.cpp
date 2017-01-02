#include "VSTPlugin.h"
#include "vst-plugin-callbacks.hpp"

#include <util/platform.h>
#include <windows.h> 

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
	plugin = mainEntryPoint(hostCallback_static);
	plugin->user = this;
	return plugin;
}

void VSTPlugin::unloadLibrary() {
	if (dllHandle) {
		FreeLibrary(dllHandle);
		dllHandle = NULL;
	}
}