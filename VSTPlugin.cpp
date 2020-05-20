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

#include "headers/VSTPlugin.h"

VSTPlugin::VSTPlugin(obs_source_t *sourceContext) : sourceContext{sourceContext}
{

	int numChannels = VST_MAX_CHANNELS;
	int blocksize   = BLOCK_SIZE;

	inputs  = (float **)malloc(sizeof(float **) * numChannels);
	outputs = (float **)malloc(sizeof(float **) * numChannels);
	for (int channel = 0; channel < numChannels; channel++) {
		inputs[channel]  = (float *)malloc(sizeof(float *) * blocksize);
		outputs[channel] = (float *)malloc(sizeof(float *) * blocksize);
	}
}

VSTPlugin::~VSTPlugin()
{
	int numChannels = VST_MAX_CHANNELS;

	for (int channel = 0; channel < numChannels; channel++) {
		if (inputs[channel]) {
			free(inputs[channel]);
			inputs[channel] = NULL;
		}
		if (outputs[channel]) {
			free(outputs[channel]);
			outputs[channel] = NULL;
		}
	}
	if (inputs) {
		free(inputs);
		inputs = NULL;
	}
	if (outputs) {
		free(outputs);
		outputs = NULL;
	}
}

void VSTPlugin::loadEffectFromPath(std::string path)
{
	if (this->pluginPath.compare(path) != 0) {
		closeEditor();
		unloadEffect();
	}

	if (!effect) {
		pluginPath = path;
		effect     = loadEffect();

		if (!effect) {
			// TODO: alert user of error
			blog(LOG_WARNING,
			     "VST Plug-in: Can't load "
			     "effect!");
			return;
		}

		// Check plug-in's magic number
		// If incorrect, then the file either was not loaded properly,
		// is not a real VST plug-in, or is otherwise corrupt.
		if (effect->magic != kEffectMagic) {
			blog(LOG_WARNING, "VST Plug-in's magic number is bad");
			return;
		}

		effect->dispatcher(effect, effGetEffectName, 0, 0, effectName, 0);
		effect->dispatcher(effect, effGetVendorString, 0, 0, vendorString, 0);

		effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0.0f);

		// Set some default properties
		size_t sampleRate = audio_output_get_sample_rate(obs_get_audio());
		effect->dispatcher(effect, effSetSampleRate, 0, 0, nullptr, sampleRate);
		int blocksize = BLOCK_SIZE;
		effect->dispatcher(effect, effSetBlockSize, 0, blocksize, nullptr, 0.0f);

		effect->dispatcher(effect, effMainsChanged, 0, 1, nullptr, 0);

		effectReady = true;

		if (openInterfaceWhenActive) {
			openEditor();
		}
	}
}

void silenceChannel(float **channelData, int numChannels, long numFrames)
{
	for (int channel = 0; channel < numChannels; ++channel) {
		for (long frame = 0; frame < numFrames; ++frame) {
			channelData[channel][frame] = 0.0f;
		}
	}
}

obs_audio_data *VSTPlugin::process(struct obs_audio_data *audio)
{
	if (effect && effectReady) {
		uint passes = (audio->frames + BLOCK_SIZE - 1) / BLOCK_SIZE;
		uint extra  = audio->frames % BLOCK_SIZE;
		for (uint pass = 0; pass < passes; pass++) {
			uint frames = pass == passes - 1 && extra ? extra : BLOCK_SIZE;
			silenceChannel(outputs, VST_MAX_CHANNELS, BLOCK_SIZE);

			float *adata[VST_MAX_CHANNELS];
			for (size_t d = 0; d < VST_MAX_CHANNELS; d++) {
				if (audio->data[d] != nullptr) {
					adata[d] = ((float *)audio->data[d]) + (pass * BLOCK_SIZE);
				} else {
					adata[d] = inputs[d];
				}
			};

			effect->processReplacing(effect, adata, outputs, frames);

			for (size_t c = 0; c < VST_MAX_CHANNELS; c++) {
				if (audio->data[c]) {
					for (size_t i = 0; i < frames; i++) {
						adata[c][i] = outputs[c][i];
					}
				}
			}
		}
	}

	return audio;
}

void VSTPlugin::unloadEffect()
{
	effectReady = false;

	if (effect) {
		effect->dispatcher(effect, effMainsChanged, 0, 0, nullptr, 0);
		effect->dispatcher(effect, effClose, 0, 0, nullptr, 0.0f);
	}

	effect = nullptr;

	unloadLibrary();
}

void VSTPlugin::openEditor()
{
	if (effect && !editorWidget) {
		editorWidget = new EditorWidget(nullptr, this);
		editorWidget->buildEffectContainer(effect);

		if (sourceName.empty()) {
			sourceName = "VST 2.x";
		}

		if (filterName.empty()) {
			editorWidget->setWindowTitle(QString("%1 - %2").arg(sourceName.c_str(), effectName));
		} else {
			editorWidget->setWindowTitle(
			        QString("%1:%2 - %3").arg(sourceName.c_str(), filterName.c_str(), effectName));
		}
		editorWidget->show();
	}
}

void VSTPlugin::closeEditor()
{
	if (effect) {
		effect->dispatcher(effect, effEditClose, 0, 0, nullptr, 0);
	}

	if (editorWidget) {
		editorWidget->close();
		delete editorWidget;
		editorWidget = nullptr;
	}
}

intptr_t VSTPlugin::hostCallback(AEffect *effect, int32_t opcode, int32_t index, intptr_t value, void *ptr, float opt)
{
	UNUSED_PARAMETER(effect);
	UNUSED_PARAMETER(ptr);
	UNUSED_PARAMETER(opt);

	intptr_t result = 0;

	// Filter idle calls...
	bool filtered = false;
	if (opcode == audioMasterIdle) {
		static bool wasIdle = false;
		if (wasIdle)
			filtered = true;
		else {
			blog(LOG_WARNING,
			     "VST Plug-in: Future idle calls "
			     "will not be displayed!");
			wasIdle = true;
		}
	}

	switch (opcode) {
	case audioMasterSizeWindow:
		// index: width, value: height
		if (editorWidget) {
			editorWidget->handleResizeRequest(index, value);
		}
		return 0;
	}

	return result;
}

std::string VSTPlugin::getChunk()
{
	if (!effect) {
		return "";
	}

	if (effect->flags & effFlagsProgramChunks) {
		void *buf = nullptr;

		intptr_t chunkSize = effect->dispatcher(effect, effGetChunk, 1, 0, &buf, 0.0);

		QByteArray data = QByteArray((char *)buf, chunkSize);
		return QString(data.toBase64()).toStdString();
	} else {
		std::vector<float> params;
		for (int i = 0; i < effect->numParams; i++) {
			float parameter = effect->getParameter(effect, i);
			params.push_back(parameter);
		}

		const char *bytes   = reinterpret_cast<const char *>(&params[0]);
		QByteArray  data    = QByteArray(bytes, (int)(sizeof(float) * params.size()));
		std::string encoded = QString(data.toBase64()).toStdString();
		return encoded;
	}
}

void VSTPlugin::setChunk(std::string data)
{
	if (!effect) {
		return;
	}

	if (effect->flags & effFlagsProgramChunks) {
		QByteArray base64Data = QByteArray(data.c_str(), (int)data.length());
		QByteArray chunkData  = QByteArray::fromBase64(base64Data);
		void *     buf        = nullptr;
		buf                   = chunkData.data();
		effect->dispatcher(effect, effSetChunk, 1, chunkData.length(), buf, 0);
	} else {
		QByteArray base64Data = QByteArray(data.c_str(), (int)data.length());
		QByteArray paramData  = QByteArray::fromBase64(base64Data);

		const char * p_chars  = paramData.data();
		const float *p_floats = reinterpret_cast<const float *>(p_chars);

		int size = paramData.length() / sizeof(float);

		std::vector<float> params(p_floats, p_floats + size);

		if (params.size() != (size_t)effect->numParams) {
			return;
		}

		for (int i = 0; i < effect->numParams; i++) {
			effect->setParameter(effect, i, params[i]);
		}
	}
}

void VSTPlugin::setProgram(const int programNumber)
{
	if (programNumber < effect->numPrograms) {
		effect->dispatcher(effect, effSetProgram, 0, programNumber, NULL, 0.0f);
	} else {
		blog(LOG_ERROR, "Failed to load program, number was outside possible program range.");
	}
}

int VSTPlugin::getProgram()
{
	return effect->dispatcher(effect, effGetProgram, 0, 0, NULL, 0.0f);
}

void VSTPlugin::getSourceNames()
{
	/* Only call inside the vst_filter_audio function! */
	sourceName = obs_source_get_name(obs_filter_get_target(sourceContext));
	filterName = obs_source_get_name(sourceContext);
}
