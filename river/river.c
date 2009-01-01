#include "river-config.h"
#include <libsummer/summer.h>
#include <loudmouth/loudmouth.h>
#include <glib.h>
#include <stdlib.h>

static GMainLoop *loop;

static gboolean daemon = FALSE;
static gint verbosity = 0;

static GOptionEntry entries[] = {
	{"daemon", 'd', 0, G_OPTION_ARG_NONE, &daemon, "Run as a daemon", NULL},
	{"verbosity", 'v', 0, G_OPTION_ARG_INT, &verbosity, "Use verbosity level N. 0 is quiet, higher numbers means more talk.", "N"},
	{NULL}};

static void
on_new_entries (SummerFeed *feed, gconstpointer user_data)
{
	RiverConfig *config = (RiverConfig *)user_data;
	gchar *url;
	GValueArray *items;
	g_object_get (feed, "url", &url, "items", &items, NULL);
	RiverSubscription *subscription = NULL;
	GList *subscriptions;
	for (subscriptions = config->subscriptions; 
			subscriptions != NULL;
			subscriptions = subscriptions->next) {
		if (!g_strcmp0(((RiverSubscription *)subscriptions->data)->url, url)) {
			subscription = (RiverSubscription *)subscriptions->data;
			break;
		}
	}
	if (subscription == NULL) {
		return;
	}

	guint i;
	for (i = 0; i < items->n_values; i++) {
		GValue *gvitem = g_value_array_get_nth (items, i);
		SummerItemData *item = g_value_get_pointer (gvitem);

		SummerDownloadableData *dlable;
		if (!item->downloadables)
			continue;
		dlable = (SummerDownloadableData *)item->downloadables->data;
		SummerDownload *dl = summer_create_download (dlable->mime, dlable->url);
		gchar *save_dir;
		if (subscription->save_dir)
			save_dir = g_strdup (subscription->save_dir);
		else
			save_dir = g_build_filename (config->save_dir, subscription->name, 
				NULL);
		g_object_set (dl, "save-dir", save_dir, NULL);
		g_free (save_dir);
		g_print ("New download starting: \"%s\" from %s\n", 
			item->title, 
			subscription->name);
		summer_download_start (dl);
	}
}

int
main (int argc, char *argv[])
{
	g_type_init ();
	g_thread_init (NULL);
	GError *error = NULL;
	GOptionContext *context;
	context = g_option_context_new ("- Download Podcasts");
	g_option_context_add_main_entries (context, entries, NULL);
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("option parsing failed: %s\n", error->message);
		exit (1);
	}
	loop = g_main_loop_new (NULL, TRUE);

	RiverConfig *config = river_load_config ();
	if (config == NULL)
		g_error ("Couldn't parse config file");
	summer_set ("download",
		"save-dir", config->save_dir,
		"tmp-dir", config->tmp_dir, NULL);
	gchar *cache = g_build_filename (g_get_user_cache_dir (), "river", NULL);
	summer_set ("feed",
		"cache-dir", cache,
		"frequency", 900, NULL);
	g_free (cache);
	GList *subscriptions;
	for (subscriptions = config->subscriptions; 
			subscriptions != NULL;
			subscriptions = subscriptions->next) {
		SummerFeed *feed = summer_feed_new ();
		g_signal_connect (feed, "new-entries", G_CALLBACK (on_new_entries), 
			config);
		summer_feed_start (feed, 
			((RiverSubscription *)subscriptions->data)->url);
	}
	g_main_loop_run (loop);
	g_main_loop_unref (loop);
	return 0;
}
