#include "river-xmpp.h"
#include <loudmouth/loudmouth.h>

#define SERVER "talk.l.google.com"
#define USERNAME "river"
#define JID "river@flukkost.nu"
#define PASSWORD "F45741"
#define RECIPIENT "ozamosi@flukkost.nu"

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
	}
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

void
river_xmpp_init ()
{
	GError *error = NULL;
	conn = lm_connection_new (SERVER);

	LmSSL *ssl = lm_ssl_new (NULL, NULL, NULL, NULL);
	lm_connection_set_ssl (conn, ssl);
	lm_connection_set_jid (conn, JID);
	lm_connection_set_port (conn, 5223);

	if (!lm_connection_open_and_block (conn, &error)) {
		g_print ("Couldn't open connection to '%s': \n%s\n", SERVER, error->message);
		g_clear_error (&error);
		return;
	}
	if (!lm_connection_authenticate_and_block (conn, USERNAME, PASSWORD, "River", &error)) {
		g_print ("Couldn't authenticate with '%s' '%s': \n%s\n", USERNAME, PASSWORD, error->message);
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
	LmMessage *m = lm_message_new (RECIPIENT, LM_MESSAGE_TYPE_MESSAGE);
	lm_message_node_add_child (m->node, "subject", subject);
	lm_message_node_add_child (m->node, "body", message);

	GError *error = NULL;
	if (!lm_connection_send (conn, m, &error)) {
		g_print ("Error sending message to '%s': \n%s\n", RECIPIENT, error->message);
	}

	lm_message_unref (m);
}

void
river_xmpp_destruct ()
{
	lm_connection_close (conn, NULL);
	lm_connection_unref (conn);
}
