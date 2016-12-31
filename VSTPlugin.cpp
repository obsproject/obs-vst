#include "VSTPlugin.h"

VSTPlugin::VSTPlugin(obs_source_t *sourceContext) : sourceContext{sourceContext} {
    int numChannels = 2;
    int blocksize = 512;

    inputs = (float **) malloc(sizeof(float **) * numChannels);
    outputs = (float **) malloc(sizeof(float **) * numChannels);
    for (int channel = 0; channel < numChannels; channel++) {
        inputs[channel] = (float *) malloc(sizeof(float *) * blocksize);
        outputs[channel] = (float *) malloc(sizeof(float *) * blocksize);
    }
}

void VSTPlugin::loadEffectFromPath(std::string path) {
    if (!effect) {
        pluginPath = path;
        effect = loadEffect();

        if (effect == NULL) {
            //TODO: alert user of error
            return;
        }

        // Check plugin's magic number
        // If incorrect, then the file either was not loaded properly, is not a
        // real VST plugin, or is otherwise corrupt.
        if (effect->magic != kEffectMagic) {
            blog(LOG_WARNING, "VST Plugin's  magic number is bad");
            return;
        }

        // Create dispatcher handle
        dispatcherFuncPtr dispatcher = (dispatcherFuncPtr)(effect->dispatcher);

        // Set up plugin callback functions
        effect->getParameter = (getParameterFuncPtr) effect->getParameter;
        effect->processReplacing = (processFuncPtr) effect->processReplacing;
        effect->setParameter = (setParameterFuncPtr) effect->setParameter;

        dispatcher(effect, effOpen, 0, 0, NULL, 0.0f);

        // Set some default properties
        size_t sampleRate = audio_output_get_sample_rate(obs_get_audio());
        dispatcher(effect, effSetSampleRate, 0, 0, NULL, sampleRate);
        int blocksize = 512;
        dispatcher(effect, effSetBlockSize, 0, blocksize, NULL, 0.0f);

        openEditor();
    } else if (this->pluginPath.compare(path) != 0) {
        closeEditor();

        //TODO: unload

    }
}

void silenceChannel(float **channelData, int numChannels, long numFrames) {
    for(int channel = 0; channel < numChannels; ++channel) {
        for(long frame = 0; frame < numFrames; ++frame) {
            channelData[channel][frame] = 0.0f;
        }
    }
}

obs_audio_data* VSTPlugin::process(struct obs_audio_data *audio) {
    silenceChannel(outputs, 2, audio->frames);

    float *adata[2] = {(float *) audio->data[0], (float *) audio->data[1]};

    effect->dispatcher(effect, effMainsChanged, 0, 1, 0, 0);

    effect->processReplacing(effect, adata, outputs, audio->frames);

    effect->dispatcher(effect, effMainsChanged, 0, 0, 0, 0);

    for (size_t c = 0; c < 2; c++) {
        if (audio->data[c]) {
            for (size_t i = 0; i < audio->frames; i++) {
                adata[c][i] = outputs[c][i];
            }
        }
    }

    return audio;
}

void VSTPlugin::openEditor() {
    if (effect) {
        editorWidget = new EditorWidget(0);

        editorWidget->buildEffectContainer(effect);

        editorWidget->show();
    }
}

void VSTPlugin::closeEditor() {
    //effect->dispatcher (effect, effEditClose, 0, 0, 0, 0);
}