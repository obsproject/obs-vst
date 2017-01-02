#include <obs-module.h>
#include <QDir>
#include <QDirIterator>

#include "VSTPlugin.h"

static bool open_editor_button_clicked(obs_properties_t *props,
                                   obs_property_t *property, void *data)
{
    UNUSED_PARAMETER(props);
    UNUSED_PARAMETER(property);
    UNUSED_PARAMETER(data);

    VSTPlugin *vstPlugin = (VSTPlugin *)data;

    vstPlugin->openEditor();

    return true;
}

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
#ifdef __APPLE__
    QDir dir("/Library/Audio/Plug-Ins/VST/");
#elif WIN32
    QDir dir("C:/Program Files/VstPlugins/");
#endif

    QStringList filters;
    filters << "*.vst" << "*.dll";
    dir.setNameFilters(filters);

    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        QString name = it.fileName();

        obs_property_list_add_string(list, name.toStdString().c_str(), path.toStdString().c_str());
    }
}

static obs_properties_t *vst_properties(void *data)
{
    obs_properties_t *props = obs_properties_create();

    obs_property_t *list = obs_properties_add_list(props, "plugin_path",
                                                   obs_module_text("VST Plugin"), OBS_COMBO_TYPE_LIST,
                                                   OBS_COMBO_FORMAT_STRING);

    fill_out_plugins(list);

    obs_properties_add_button(props, "editor",
                              obs_module_text("Open Effect Editor"), open_editor_button_clicked);

    UNUSED_PARAMETER(data);
    return props;
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-vst", "en-US")

bool obs_module_load(void)
{
	struct obs_source_info vst_filter = {};
	vst_filter.id = "vst_filter";
	vst_filter.type = OBS_SOURCE_TYPE_FILTER;
	vst_filter.output_flags = OBS_SOURCE_AUDIO;
	vst_filter.get_name = vst_name;
	vst_filter.create = vst_create;
	vst_filter.destroy = vst_destroy;
	vst_filter.update = vst_update;
	vst_filter.filter_audio = vst_filter_audio;
	vst_filter.get_defaults = vst_defaults;
	vst_filter.get_properties = vst_properties;

    obs_register_source(&vst_filter);
    return true;
}