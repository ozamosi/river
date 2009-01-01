#include "river-config.h"

/*
 * Example configuration file:
 * 
 * <river>
 *  <save_dir>/home/username/mediafiles/</save_dir>
 *  <tmp_dir>/home/username/.cache/</tmp_dir>
 *  <subscription>
 *   <name>Awesome feed</name>
 *   <url>http://awesome.com/feed</url>
 *   <save_dir>(optional)/home/username/other_mediafiles_dir</save_dir>
 *  </subscription>
 * </river>
 */


struct parse_context {
	RiverConfig *config;
	gboolean subscription;
	gboolean save_dir;
	gboolean tmp_dir;
	gboolean name;
	gboolean url;
};

static void
start_element (GMarkupParseContext *context,
		const gchar *element_name,
		const gchar **attribute_names,
		const gchar **attribute_values,
		gpointer user_data,
		GError **error)
{
	struct parse_context *data = (struct parse_context *) user_data;
	if (!g_strcmp0 (element_name, "save_dir"))
		data->save_dir = TRUE;
	else if (!g_strcmp0 (element_name, "tmp_dir"))
		data->tmp_dir = TRUE;
	else if (!g_strcmp0 (element_name, "subscription")) {
		data->config->subscriptions = g_list_prepend (
				data->config->subscriptions, 
				g_slice_new0 (RiverSubscription));
		data->subscription = TRUE;
	} else if (!g_strcmp0 (element_name, "name"))
		data->name = TRUE;
	else if (!g_strcmp0 (element_name, "url"))
		data->url = TRUE;
}

static void
text (GMarkupParseContext *context,
	const gchar *text,
	gsize text_len,
	gpointer user_data,
	GError **error)
{
	struct parse_context *data = (struct parse_context *) user_data;
	if (data->save_dir && !data->subscription)
		data->config->save_dir = g_strdup (text);
	else if (data->tmp_dir && !data->subscription)
		data->config->tmp_dir = g_strdup (text);
	else if (data->name && data->subscription)
		((RiverSubscription *)data->config->subscriptions->data)->name = g_strdup (text);
	else if (data->url && data->subscription)
		((RiverSubscription *)data->config->subscriptions->data)->url = g_strdup (text);
	else if (data->save_dir && data->subscription)
		((RiverSubscription *)data->config->subscriptions->data)->save_dir = g_strdup (text);
}

static void
end_element (GMarkupParseContext *context,
	const gchar *element_name,
	gpointer user_data,
	GError **error)
{
	struct parse_context *data = (struct parse_context *) user_data;
	if (!g_strcmp0 (element_name, "save_dir"))
		data->save_dir = FALSE;
	else if (!g_strcmp0 (element_name, "tmp_dir"))
		data->tmp_dir = FALSE;
	else if (!g_strcmp0 (element_name, "subscription"))
		data->subscription = FALSE;
	else if (!g_strcmp0 (element_name, "name"))
		data->name = FALSE;
	else if (!g_strcmp0 (element_name, "url"))
		data->url = FALSE;
}	

static GMarkupParser parser = {
	start_element,
	end_element,
	text,
	NULL,
	NULL
};

RiverConfig*
river_load_config ()
{
	gchar *config_path = g_build_filename (
		g_get_user_config_dir (), "river", "config.xml", NULL);
	struct parse_context *data = g_slice_new0 (struct parse_context);
	data->config = g_slice_new0 (RiverConfig);
	GMarkupParseContext *context = g_markup_parse_context_new (
		&parser,
		0,
		data,
		NULL);
	gchar *text;
	gsize length;
	GError *error = NULL;
	if (g_file_get_contents (config_path, &text, &length, &error)) {
		g_markup_parse_context_parse (context, text, length, &error);
		g_markup_parse_context_end_parse (context, NULL);
	}
	g_markup_parse_context_free (context);
	RiverConfig *config = data->config;
	g_slice_free (struct parse_context, data);
	return config;
}
