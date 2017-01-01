#include <obs-module.h>
#include <util/platform.h>

#include "VSTPlugin.h"

static const char *vst_name(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("VST");
}

static void vst_destroy(void *data)
{
    VSTPlugin *vstPlugin = (VSTPlugin *)data;

    vstPlugin->closeEditor();

    delete vstPlugin;
}

static void vst_update(void *data, obs_data_t *settings)
{
    VSTPlugin *vstPlugin = (VSTPlugin *)data;

    const char *path = obs_data_get_string(settings, "plugin_path");

    if (strcmp(path, "") == 0) {
        return;
    }
    vstPlugin->loadEffectFromPath(std::string(path));
}

static void *vst_create(obs_data_t *settings, obs_source_t *filter)
{
    VSTPlugin *vstPlugin = new VSTPlugin(filter);

    vst_update(vstPlugin, settings);
    return vstPlugin;
}

static struct obs_audio_data *vst_filter_audio(void *data,
                                                struct obs_audio_data *audio)
{
    VSTPlugin *vstPlugin = (VSTPlugin *)data;

    vstPlugin->process(audio);

    return audio;
}

static void vst_defaults(obs_data_t *s)
{

}

static void fill_out_plugins(obs_property_t *list)
{
    os_glob_t *glob;

    // TODO: make platform glob better or do someting else since this globstar won't work
    //if (os_glob("/Library/Audio/Plug-Ins/VST/*.vst", 0, &glob) != 0) {
    if (os_glob("/Library/Audio/Plug-Ins/VST/MeldaProduction/Tools/MOscilloscope.vst", 0, &glob) != 0) {
        blog(LOG_WARNING, "Failed to glob VST plugins");
        return;
    }

    for (size_t i = 0; i < glob->gl_pathc; i++) {
        const char *filePath = glob->gl_pathv[i].path;

        obs_property_list_add_string(list,
                                     filePath,
                                     filePath);
    }

    os_globfree(glob);
}

static obs_properties_t *vst_properties(void *data)
{
    obs_properties_t *props = obs_properties_create();

    obs_property_t *list = obs_properties_add_list(props, "plugin_path",
                                                   obs_module_text("VST Plugin"), OBS_COMBO_TYPE_LIST,
                                                   OBS_COMBO_FORMAT_STRING);

    fill_out_plugins(list);

    UNUSED_PARAMETER(data);
    return props;
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-vst", "en-US")

struct obs_source_info vst_filter = {
        .id = "vst_filter",
        .type = OBS_SOURCE_TYPE_FILTER,
        .output_flags = OBS_SOURCE_AUDIO,
        .get_name = vst_name,
        .create = vst_create,
        .destroy = vst_destroy,
        .update = vst_update,
        .filter_audio = vst_filter_audio,
        .get_defaults = vst_defaults,
        .get_properties = vst_properties,
};

bool obs_module_load(void)
{
    obs_register_source(&vst_filter);
    return true;
}