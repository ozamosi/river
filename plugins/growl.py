#!/usr/bin/env python

###
### netgrowl
###
"""Growl 0.6 Network Protocol Client for Python"""
__version__ = "0.6.1" # will always match Growl version
__author__ = "Rui Carmo (http://the.taoofmac.com)"
__copyright__ = "(C) 2004 Rui Carmo. Code under BSD License."
__contributors__ = "Ingmar J Stein (Growl Team)"

import struct
import md5
from socket import AF_INET, SOCK_DGRAM, socket

GROWL_UDP_PORT=9887
GROWL_PROTOCOL_VERSION=1
GROWL_TYPE_REGISTRATION=0
GROWL_TYPE_NOTIFICATION=1

class GrowlRegistrationPacket:
  """Builds a Growl Network Registration packet.
     Defaults to emulating the command-line growlnotify utility."""

  def __init__(self, application="growlnotify", password = None ):
    self.notifications = []
    self.defaults = [] # array of indexes into notifications
    self.application = application.encode("utf-8")
    self.password = password
  # end def


  def addNotification(self, notification="Command-Line Growl Notification", enabled=True):
    """Adds a notification type and sets whether it is enabled on the GUI"""
    self.notifications.append(notification)
    if enabled:
      self.defaults.append(len(self.notifications)-1)
  # end def


  def payload(self):
    """Returns the packet payload."""
    self.data = struct.pack( "!BBH",
                             GROWL_PROTOCOL_VERSION,
                             GROWL_TYPE_REGISTRATION,
                             len(self.application) )
    self.data += struct.pack( "BB",
                              len(self.notifications),
                              len(self.defaults) )
    self.data += self.application
    for notification in self.notifications:
      encoded = notification.encode("utf-8")
      self.data += struct.pack("!H", len(encoded))
      self.data += encoded
    for default in self.defaults:
      self.data += struct.pack("B", default)
    self.checksum = md5.new()
    self.checksum.update(self.data)
    if self.password:
       self.checksum.update(self.password)
    self.data += self.checksum.digest()
    return self.data
  # end def
# end class


class GrowlNotificationPacket:
  """Builds a Growl Network Notification packet.
     Defaults to emulating the command-line growlnotify utility."""

  def __init__(self, application="growlnotify",
               notification="Command-Line Growl Notification", title="Title",
               description="Description", priority = 0, sticky = False, password = None ):
    self.application  = application.encode("utf-8")
    self.notification = notification.encode("utf-8")
    self.title        = title.encode("utf-8")
    self.description  = description.encode("utf-8")
    flags = (priority & 0x07) * 2
    if priority < 0:
      flags |= 0x08
    if sticky:
      flags = flags | 0x0100
    self.data = struct.pack( "!BBHHHHH",
                             GROWL_PROTOCOL_VERSION,
                             GROWL_TYPE_NOTIFICATION,
                             flags,
                             len(self.notification),
                             len(self.title),
                             len(self.description),
                             len(self.application) )
    self.data += self.notification
    self.data += self.title
    self.data += self.description
    self.data += self.application
    self.checksum = md5.new()
    self.checksum.update(self.data)
    if password:
       self.checksum.update(password)
    self.data += self.checksum.digest()
  # end def

  def payload(self):
    """Returns the packet payload."""
    return self.data
  # end def
# end class

###
### End netgrowl
###


# Actual plugin
# config: str: ip; str: password;


def notify(pdata, notif_type, title, message):
    p = GrowlNotificationPacket (application=pdata['app'], notification=notif_type,
                                title=title, description=message, priority=1,
                                sticky=True, password=pdata['config']['password'])
    pdata['socket'].sendto (p.payload (), pdata['addr'])

def new_entry (pdata, dl, item): # not of interest for this plugin, but needs to be implemented
    pass

def download_complete (pdata, dl, item):
    notify (pdata, "Download Complete", 'Download Complete', item['title'])

def download_started (pdata, dl, item):
    notify (pdata, "Download Started", 'Download Started', item['title'])

def download_update (pdata, dl, downloaded, length, item):
    pass

def init (pdata):
    pdata['app'] = 'river'
    pdata['addr'] = (pdata['config']['ip'], GROWL_UDP_PORT)
    pdata['socket'] = socket (AF_INET, SOCK_DGRAM)
    p = GrowlRegistrationPacket (application=pdata['app'], password=pdata['config']['password'])
    p.addNotification ("Download Started", enabled=True)
    p.addNotification ("Download Complete", enabled=True)
    pdata['socket'].sendto (p.payload (), pdata['addr'])
    print 'Loaded growl plugin'

def shutdown (pdata):
    pdata['socket'].close ()
    print 'Unloaded growl plugin'
