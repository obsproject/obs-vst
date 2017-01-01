#ifndef OBS_STUDIO_VSTPLUGIN_H
#define OBS_STUDIO_VSTPLUGIN_H

#include <string>
#include "aeffectx.h"
#include <obs-module.h>
#include "vst-plugin-callbacks.hpp"
#include "EditorWidget.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

class VSTPlugin {
    AEffect *effect = NULL;
    obs_source_t *sourceContext;
    std::string pluginPath;

    float **inputs;
    float **outputs;

    EditorWidget *editorWidget = NULL;

    AEffect* loadEffect();

    bool effectReady = false;

#ifdef __APPLE__
    CFBundleRef bundle = NULL;
#elif WIN32

#endif

public:
    VSTPlugin(obs_source_t *sourceContext);
    void loadEffectFromPath(std::string path);
    void unloadEffect();
    void openEditor();
    void closeEditor();
    obs_audio_data* process(struct obs_audio_data *audio);
};


#endif //OBS_STUDIO_VSTPLUGIN_H
