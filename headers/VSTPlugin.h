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

class EditorWidget;

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
		HINSTANCE dllHandle = NULL;
	#endif

	void unloadLibrary();

	static intptr_t hostCallback_static( AEffect *effect, int32_t opcode,
			int32_t index, intptr_t value, void *ptr, float opt)
	{
		if (effect && effect->user) {
		auto* plugin = static_cast<VSTPlugin*>(effect->user);
		return plugin->hostCallback(effect, opcode, index, value, ptr, opt);
	}

	switch (opcode)
	{
		case audioMasterVersion:
		return (intptr_t) 2400;

		default:
			return 0;
		}
	}

	intptr_t hostCallback( AEffect *effect, int32_t opcode, int32_t index,
			intptr_t value, void *ptr, float opt);

	public:
		VSTPlugin(obs_source_t *sourceContext);
		void loadEffectFromPath(std::string path);
		void unloadEffect();
		void openEditor();
		void closeEditor();
		std::string getChunk();
		void setChunk(std::string data);
		obs_audio_data* process(struct obs_audio_data *audio);
};

#endif //OBS_STUDIO_VSTPLUGIN_H
