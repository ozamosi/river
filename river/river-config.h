#include <glib.h>

typedef struct _RiverConfig RiverConfig;
typedef struct _RiverSubscription RiverSubscription;

RiverConfig* river_load_config (void);
RiverConfig* river_save_config (void);

struct _RiverConfig {
	gint frequency;
	gchar *save_dir;
	gchar *tmp_dir;
	GList *subscriptions;
};

struct _RiverSubscription {
	gchar *name;
	gchar *url;
	gchar *save_dir;
};
