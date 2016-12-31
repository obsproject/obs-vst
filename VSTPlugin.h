#ifndef OBS_STUDIO_VSTPLUGIN_H
#define OBS_STUDIO_VSTPLUGIN_H

#include <string>
#include "aeffectx.h"
#include <obs-module.h>
#include "vst-plugin-callbacks.hpp"
#include "EditorWidget.h"

class VSTPlugin {
    AEffect *effect = NULL;
    obs_source_t *sourceContext;
    std::string pluginPath;

    float **inputs;
    float **outputs;

    EditorWidget *editorWidget;

    AEffect* loadEffect();

public:
    VSTPlugin(obs_source_t *sourceContext);
    void loadEffectFromPath(std::string path);
    void openEditor();
    void closeEditor();
    obs_audio_data* process(struct obs_audio_data *audio);
};


#endif //OBS_STUDIO_VSTPLUGIN_H
