#include "river-config.h"
#include "river-xmpp.h"

#include <libsummer/summer.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

static GMainLoop *loop;

static gboolean daemonize;
static gchar *server;
static gchar *username;
static gchar *jid;
static gchar *password;
static gchar *recipient;
static gchar *config_path;
static gboolean no_cache;
static gboolean ssl;
static gint port;

static GOptionEntry entries[] = {
	{"daemon", 'd', 0, G_OPTION_ARG_NONE, &daemonize, "Run as a daemon", NULL},
	{"server", 's', 0, G_OPTION_ARG_STRING, &server, "XMPP Server to connect to", NULL},
	{"username", 'u', 0, G_OPTION_ARG_STRING, &username, "River's username", NULL},
	{"jid", 'j', 0, G_OPTION_ARG_STRING, &jid, "River's JabberID (usually username@server)", NULL},
	{"password", 'p', 0, G_OPTION_ARG_STRING, &password, "River's XMPP password", NULL},
	{"recipient", 'r', 0, G_OPTION_ARG_STRING, &recipient, "The recipient of any notifications", NULL},
	{"ssl", 'l', 0, G_OPTION_ARG_NONE, &ssl, "Whether to use SSL when connecting", NULL},
	{"port", 'o', 0, G_OPTION_ARG_INT, &port, "Port to connect to", NULL},
	{"config", 'c', 0, G_OPTION_ARG_FILENAME, &config_path, "File to use as configuration file, if other than the default", NULL},
	{"no-cache", 0, 0, G_OPTION_ARG_NONE, &no_cache, "Disable the use of a cache file. The cache file prevents redownloads", NULL},
	{NULL}};

static void
signal_handler (int signum)
{
	river_xmpp_destruct ();
	exit (0);
}

static void
on_download_complete (SummerDownload *dl, gchar *save_path, gconstpointer user_data)
{
	gchar *message = g_strdup_printf ("Saved to '%s'", save_path);
	river_xmpp_send ("Download complete", message);
	g_free (message);
}

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
		gchar *subject = g_strdup_printf ("New download starting from %s", subscription->name);
		gchar *message = g_strdup_printf ("%s: %s", 
			subscription->name, item->title);
		river_xmpp_send (subject, message);
		g_free (message);
		g_free (subject);
		g_signal_connect (dl, "download-complete", G_CALLBACK (on_download_complete), NULL);
		summer_download_start (dl);
	}
}

int
main (int argc, char *argv[])
{
	signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
	
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

	if (server == NULL)
		g_error ("You didn't say what XMPP server you want to connect to");
	if (username == NULL)
		g_error ("You didn't say what XMPP username to connect as");
	if (password == NULL)
		g_error ("You didn't say what XMPP password to use");
	if (!port)
		port = 5222;

	RiverConfig *config = river_load_config (config_path);
	river_xmpp_init (server, username, jid, password, recipient, ssl, port);
	if (config == NULL)
		g_error ("Couldn't parse config file");
	
	if (!config->frequency)
		config->frequency = 900;

	summer_set ("download",
		"save-dir", config->save_dir,
		"tmp-dir", config->tmp_dir, NULL);
	if (!no_cache) {
		gchar *cache = g_build_filename (
			g_get_user_cache_dir (), 
			"river", 
			NULL);
		summer_set ("feed", "cache-dir", cache, NULL);
		g_free (cache);
	}
	summer_set ("feed", "frequency", config->frequency, NULL);
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
	if (daemonize) {
		pid_t pid = fork ();
		if (pid < 0)
			exit (1);
		else if (pid > 0)
			exit (0);
		
		if (setsid () < 0)
			exit (1);

		pid = fork ();
		if (pid < 0)
			exit (1);
		else if (pid > 0)
			exit (0);

		if (chdir ("/") < 0)
			exit (1);
		
		umask (0);

		close (0);
		close (1);
		close (2);
	}

	g_main_loop_run (loop);
	g_main_loop_unref (loop);
	return 0;
}
