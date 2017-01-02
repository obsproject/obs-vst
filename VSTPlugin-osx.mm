#include "VSTPlugin.h"

AEffect* VSTPlugin::loadEffect() {
    AEffect* newEffect = NULL;

    // Create a path to the bundle
    CFStringRef pluginPathStringRef = CFStringCreateWithCString(NULL, pluginPath.c_str(), kCFStringEncodingUTF8);
    CFURLRef bundleUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                       pluginPathStringRef, kCFURLPOSIXPathStyle, true);
    if (bundleUrl == NULL) {
        printf("Couldn't make URL reference for plugin\n");
        return NULL;
    }

    // Open the bundle
    bundle = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
    if (bundle == NULL) {
        printf("Couldn't create bundle reference\n");
        CFRelease(pluginPathStringRef);
        CFRelease(bundleUrl);
        return NULL;
    }

    vstPluginFuncPtr mainEntryPoint = NULL;
    mainEntryPoint = (vstPluginFuncPtr) CFBundleGetFunctionPointerForName(bundle,
                                                                          CFSTR("VSTPluginMain"));
    // VST plugins previous to the 2.4 SDK used main_macho for the entry point name
    if (mainEntryPoint == NULL) {
        mainEntryPoint = (vstPluginFuncPtr) CFBundleGetFunctionPointerForName(bundle,
                                                                              CFSTR("main_macho"));
    }

    if (mainEntryPoint == NULL) {
        printf("Couldn't get a pointer to plugin's main()\n");
        CFBundleUnloadExecutable(bundle);
        CFRelease(bundle);
        return NULL;
    }

    newEffect = mainEntryPoint(hostCallback_static);
    if (newEffect == NULL) {
        printf("Plugin's main() returns null\n");
        CFBundleUnloadExecutable(bundle);
        CFRelease(bundle);
        return NULL;
    }
    newEffect->user = this;

    // Clean up
    CFRelease(pluginPathStringRef);
    CFRelease(bundleUrl);

    return newEffect;
}

void VSTPlugin::unloadLibrary() {
    if (bundle) {
        CFBundleUnloadExecutable(bundle);
        CFRelease(bundle);
    }
}
