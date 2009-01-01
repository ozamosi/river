#include <glib.h>

#ifndef __RIVER_XMPP_H__
#define __RIVER_XMPP_H__

void river_xmpp_init (void);
void river_xmpp_send (gchar *subject, gchar *message);
void river_xmpp_destruct (void);

#endif
