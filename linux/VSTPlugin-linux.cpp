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

	wchar_t *wpath;
	os_utf8_to_wcs_ptr(pluginPath.c_str(), 0, &wpath);
	soHandle = LoadLibraryW(wpath);
	bfree(wpath);
	if(soHandle == nullptr) {
		printf("Failed trying to load VST from '%s', error %d\n",
			pluginPath, GetLastError());
		return nullptr;
	}

	vstPluginMain mainEntryPoint =
		(vstPluginMain)GetProcAddress(soHandle, "VSTPluginMain");

	if (!mainEntryPoint) {
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