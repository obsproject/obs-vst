#include "VSTPlugin.h"

#include <CoreFoundation/CoreFoundation.h>

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
        case audioMasterVersion :
            result = kVstVersion;
            break;
    }

    return result;
}

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
    CFBundleRef bundle;
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

    newEffect = mainEntryPoint(HostCallback);
    if (newEffect == NULL) {
        printf("Plugin's main() returns null\n");
        CFBundleUnloadExecutable(bundle);
        CFRelease(bundle);
        return NULL;
    }

    // Clean up
    CFRelease(pluginPathStringRef);
    CFRelease(bundleUrl);

    return newEffect;
}
