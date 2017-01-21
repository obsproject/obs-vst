#include <obs-module.h>
#include <QDir>
#include <QDirIterator>

#include "headers/VSTPlugin.h"

#define OPEN_VST_SETTINGS               "open_vst_settings"
#define CLOSE_VST_SETTINGS              "close_vst_settings"

#define OPEN_VST_TEXT                    obs_module_text("Open Plug-in Interface")
#define CLOSE_VST_TEXT                   obs_module_text("Close Plug-in Interface")

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-vst", "en-US")

static bool open_editor_button_clicked(obs_properties_t *props,
		 obs_property_t *property, void *data)
{
	VSTPlugin *vstPlugin = (VSTPlugin *)data;

	vstPlugin->openEditor();

	obs_property_set_visible(obs_properties_get(props,
			OPEN_VST_SETTINGS), false);
	obs_property_set_visible(obs_properties_get(props,
			CLOSE_VST_SETTINGS), true);

	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	UNUSED_PARAMETER(data);

	return true;
}

static bool close_editor_button_clicked(obs_properties_t *props,
		 obs_property_t *property, void *data)
{
	VSTPlugin *vstPlugin = (VSTPlugin *)data;

	vstPlugin->closeEditor();

	obs_property_set_visible(obs_properties_get(props,
			OPEN_VST_SETTINGS), true);
	obs_property_set_visible(obs_properties_get(props,
			CLOSE_VST_SETTINGS), false);

	UNUSED_PARAMETER(property);

	return true;
}

static const char *vst_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("VST Plug-in");
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

	if (strcmp(path, "") == 0)
	{
		return;
	}
	vstPlugin->loadEffectFromPath(std::string(path));

	const char *chunkData = obs_data_get_string(settings, "chunk_data");
	if (chunkData && strlen(chunkData) > 0) {
		vstPlugin->setChunk(std::string(chunkData));
		obs_data_set_string(settings, "chunk_data",
				vstPlugin->getChunk().c_str());
	}
}

static void *vst_create(obs_data_t *settings, obs_source_t *filter)
{
	VSTPlugin *vstPlugin = new VSTPlugin(filter);
	vst_update(vstPlugin, settings);

	return vstPlugin;
}

static void vst_save(void *data, obs_data_t *settings)
{
	VSTPlugin *vstPlugin = (VSTPlugin *)data;

	obs_data_set_string(settings, "chunk_data", vstPlugin->getChunk().c_str());
}

static struct obs_audio_data *vst_filter_audio(void *data,
		struct obs_audio_data *audio)
{
	VSTPlugin *vstPlugin = (VSTPlugin *)data;
	vstPlugin->process(audio);

	return audio;
}

static void fill_out_plugins(obs_property_t *list)
{
	QStringList dir_list;

	#ifdef __APPLE__
		dir_list << "/Library/Audio/Plug-Ins/VST/";
		dir_list << "~/Library/Audio/Plug-ins/VST/";
	#elif WIN32
		dir_list << "C:/Program Files/Steinberg/VstPlugins/"
		<< "C:/Program Files/Common Files/Steinberg/Shared Components/"
		<< "C:/Program Files/Common Files/VST2"
		<< "C:/Program Files/Common Files/VSTPlugins/"
		<< "C:/Program Files/VSTPlugins/";
		// If VST3 support is added....
		// << "C:/Program Files/Common Files/VST3";
	#elif LINUX
		dir_list << "/usr/lib/vst"
		dir_list << "/usr/lib/lxvst"
		dir_list << "/usr/local/lib/vst"
		dir_list << "/usr/local/lib/lxvst"
		dir_list << "~/.vst"
		dir_list << "~/.lxvst";
	#endif

	QStringList filters;
	filters << "*.vst" << "*.dll" << "*.so";

	QStringList vst_list;

	// Read all plugins into a list...
	for (int a = 0; a < dir_list.size(); ++a)
	{
		QDir search_dir(dir_list[a]);
		search_dir.setNameFilters(filters);
		QDirIterator it(search_dir, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString path = it.next();
			QString name = it.fileName();
			name.remove(QRegExp("(\\.dll|\\.vst|\\.so|\\.o)"));
			name.append("=").append(path);
			vst_list << name;
		}
	}

	// Now sort list alphabetically.
	qStableSort(vst_list.begin(), vst_list.end());

	// Now add said list to the plug-in list of OBS
	obs_property_list_add_string(list, "{Please select a plug-in}", nullptr);
	for (int b = 0; b < vst_list.size(); ++b)
	{
		QString vst_sorted = vst_list[b];
		obs_property_list_add_string(list,
				vst_sorted.left(vst_sorted.indexOf('=')).toStdString().c_str(),
				vst_sorted.mid(vst_sorted.indexOf('=') + 1).toStdString().c_str());
	}
}

static obs_properties_t *vst_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();
	obs_property_t *list = obs_properties_add_list(props, "plugin_path",
			obs_module_text("VST Plug-in"), OBS_COMBO_TYPE_LIST,
			OBS_COMBO_FORMAT_STRING);

	fill_out_plugins(list);

	obs_properties_add_button(props, OPEN_VST_SETTINGS, OPEN_VST_TEXT,
			open_editor_button_clicked);

	obs_properties_add_button(props, CLOSE_VST_SETTINGS, CLOSE_VST_TEXT,
			close_editor_button_clicked);
	obs_property_set_visible(obs_properties_get(props, CLOSE_VST_SETTINGS),
			false);

	UNUSED_PARAMETER(data);

	return props;
}

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
	vst_filter.get_properties = vst_properties;
	vst_filter.save = vst_save;

	obs_register_source(&vst_filter);
	return true;
}