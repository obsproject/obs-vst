/*****************************************************************************
Copyright (C) 2016-2017 by Colin Edwards.
Additional Code Copyright (C) 2016-2017 by c3r1c3 <c3r1c3@nevermindonline.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/
#include "../headers/VSTPlugin.h"

#include <util/platform.h>
#include <X11/Xlib.h>

AEffect* VSTPlugin::loadEffect() {
	AEffect *plugin = nullptr;

	// Why is this a (non-portable) wide char?
	wchar_t *wpath = NULL;

	// We need to convert the above wide char to a 'normal' char
	// 4096 elements should be enough to contain it... I hope.
	// We also initialize the array with NULLs to prevent issues
	// down the road.
	char charPath[4096] = {NULL};
	uint32_t wcs_returnValue;
	//os_utf8_to_wcs_ptr(pluginPath.c_str(), 0, &wpath);
	wcs_returnValue = wcstombs(charPath, wpath, sizeof(charPath));

	// Now check for errors in the conversion before trying to open
	// the file/path.
	if (wcs_returnValue < 4) {
		wprintf(L"Path conversion error from '%s'", wpath);
		printf(", to '%s'\n", charPath);
		return nullptr;
	}

	soHandle = os_dlopen(charPath);
	bfree(wpath);
	bfree(charPath);
	if(soHandle == nullptr) {
		printf("Failed trying to load VST from '%s', error %d\n",
		       pluginPath.c_str(), errno);
		return nullptr;
	}

	vstPluginMain mainEntryPoint =
			(vstPluginMain)(soHandle, "VSTPluginMain");

	if (mainEntryPoint == nullptr) {
		mainEntryPoint =
				(vstPluginMain)os_dlsym(soHandle, "VstPluginMain()");
	}

	if (mainEntryPoint == nullptr) {
		mainEntryPoint = (vstPluginMain)os_dlsym(soHandle, "main");
	}

	if (mainEntryPoint == nullptr) {
		printf("Couldn't get a pointer to plugin's main()");
		return nullptr;
	}

	// Instantiate the plug-in
	plugin = mainEntryPoint(hostCallback_static);
	plugin->user = this;
	return plugin;
}

void VSTPlugin::unloadLibrary() {
	if (soHandle) {
		os_dlclose(soHandle);
		soHandle = nullptr;
	}
}