#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import xmpp
import sys

class Xmpp(object):
    def __init__ (self, jid, password, recipient, port = 5222, server = None):
        self.recipient = recipient
        jid = xmpp.protocol.JID (jid)
        if not server:
            server = jid.getDomain ()
        self.client = xmpp.client.Client (server = server, port = port, debug = [])

        self.connection = self.client.connect ((server, port))
        if not self.connection:
            sys.stderr.write ("Could not connect")
            return
        print "Connected with %s" % self.connection
        auth = self.client.auth (jid.getNode (), password, resource = jid.getResource ())
        if not auth:
            sys.stderr.write ("Could not authenticate")
            return
        print "Authenticated using %s" % auth

        self.client.sendInitPresence ()
        self.client.RegisterDisconnectHandler (self.client.reconnectAndReauth)

    def send (self, subject, message):
        print "= %s =\n%s" % (subject, message)
        if not self.client.isConnected ():
            self.client.reconnectAndReauth ()
        msg = xmpp.protocol.Message (to = self.recipient, body = message, subject = subject, typ = 'chat')
        self.client.send (msg)

    loop_counter = 0
    max_counter = 60
    def loop (self):
        self.client.Process (1)
        self.loop_counter += 1
        if self.loop_counter >= self.max_counter:
            self.loop_counter = 0
            self.client.send (' ')

def xmpp_process (xmpp):
    xmpp.loop ()
    return True
