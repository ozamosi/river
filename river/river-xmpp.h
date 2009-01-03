#include <glib.h>

#ifndef __RIVER_XMPP_H__
#define __RIVER_XMPP_H__

void river_xmpp_init (gchar *server, gchar *username, gchar *jid, gchar *password, gchar *recipient_, gboolean ssl, gint port);
void river_xmpp_send (gchar *subject, gchar *message);
void river_xmpp_destruct (void);

#endif
