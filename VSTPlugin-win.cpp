#include "VSTPlugin.h"
#include "vst-plugin-callbacks.hpp"

AEffect* VSTPlugin::loadEffect() {
    AEffect *plugin = NULL;

    modulePtr = LoadLibrary(vstPluginPath);
    if(modulePtr == NULL) {
        printf("Failed trying to load VST from '%s', error %d\n",
               vstPath, GetLastError());
        return NULL;
    }

    vstPluginFuncPtr mainEntryPoint =
            (vstPluginFuncPtr)GetProcAddress(modulePtr, "VSTPluginMain");
    // Instantiate the plugin
    plugin = mainEntryPoint(hostCallback);
    return plugin;
}