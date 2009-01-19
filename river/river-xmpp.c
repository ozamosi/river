#include "river-xmpp.h"
#include <glib/gprintf.h>
#include <loudmouth/loudmouth.h>

static gchar *recipient;
static LmConnection *conn;

static LmHandlerResult
auto_subscribe (LmMessageHandler *handler, LmConnection *connection, 
		LmMessage *message, gpointer user_data) 
{
	if (lm_message_get_sub_type (message) == LM_MESSAGE_SUB_TYPE_SUBSCRIBE) {
		const gchar *sender = lm_message_node_get_attribute (message->node, "from");
		LmMessage *reply = lm_message_new_with_sub_type (sender,
				LM_MESSAGE_TYPE_PRESENCE,
				LM_MESSAGE_SUB_TYPE_SUBSCRIBED);

		if (!lm_connection_send (conn, reply, NULL)) {
			g_print ("Error sending message to '%s'\n", sender);
		}
		lm_message_unref (reply);
	} else {
		g_print ("Received jabber message of type %i and subtype %i\n", lm_message_get_type (message), lm_message_get_sub_type (message));
	}
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

void
river_xmpp_init (gchar *server, gchar *username, gchar *jid, gchar *password, gchar *recipient_, gboolean ssl, gint port)
{
	if (jid == NULL)
		g_sprintf (jid, "%s@%s", username, server);
	recipient = recipient_;
	GError *error = NULL;
	conn = lm_connection_new (server);

	LmSSL *lmssl = lm_ssl_new (NULL, NULL, NULL, NULL);
	if (ssl)
		lm_connection_set_ssl (conn, lmssl);
	lm_connection_set_jid (conn, jid);
	lm_connection_set_port (conn, port);

	if (!lm_connection_open_and_block (conn, &error)) {
		g_print ("Couldn't open connection to '%s': \n%s\n", server, error->message);
		g_clear_error (&error);
		return;
	}
	if (!lm_connection_authenticate_and_block (conn, username, password, "River", &error)) {
		g_print ("Couldn't authenticate with '%s' '%s': \n%s\n", username, password, error->message);
		g_clear_error (&error);
		return;
	}
	LmMessageHandler *handler;
	handler = lm_message_handler_new (auto_subscribe, NULL, NULL);
	lm_connection_register_message_handler (conn, handler, LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
	lm_message_handler_unref (handler);

	LmMessage *m = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
	lm_connection_send (conn, m, NULL);
	lm_message_unref (m);
}

void
river_xmpp_send (gchar *subject, gchar *message)
{
	if (!recipient)
		return;
	LmMessage *m = lm_message_new (recipient, LM_MESSAGE_TYPE_MESSAGE);
	lm_message_node_add_child (m->node, "subject", subject);
	lm_message_node_add_child (m->node, "body", message);

	GError *error = NULL;
	if (!lm_connection_send (conn, m, &error)) {
		g_print ("Error sending message to '%s': \n%s\n", recipient, error->message);
	}

	lm_message_unref (m);
}

void
river_xmpp_destruct ()
{
	if (conn != NULL) {
		if (lm_connection_is_open (conn))
			lm_connection_close (conn, NULL);
		lm_connection_unref (conn);
	}
}
