#!/usr/bin/env python
# -*- encoding: utf-8 -*-
import summer
import river_config
import river_xmpp
import sys
import os.path
from optparse import OptionParser
try:
    import glib
except ImportError:
    import gobject as glib

parser = OptionParser ()
parser.add_option ("-j", "--jid", dest = "jabber_id", help = "The Jabber ID to use for logging in")
parser.add_option ("-p", "--password", dest = "password", help = "The password to use for logging in")
parser.add_option ("-r", "--recipient", dest = "recipient", help = "The Jabber ID to send updates to")
parser.add_option ("--port", dest = "port", help = "Port to connect to. Defaults to 5222. Other common values are 5223 for SSL connections")
parser.add_option ("-s", "--server", dest = "server", help = "The server to connect to (normally not needed)", default = None)
parser.add_option ("--no-cache", dest = "no_cache", help = "Disable the cache file, forcing River to download everything it sees", action = "store_true", default = False)
parser.add_option ("-c", "--config", dest = "config", help = "Use a custom config file instead the default", default = None)

(options, args) = parser.parse_args ()

config = river_config.Config (options.config).result

if options.jabber_id:
    xmpp = river_xmpp.Xmpp (options.jabber_id, options.password, options.recipient, options.port, options.server)
    glib.timeout_add (5000, river_xmpp.xmpp_process, xmpp)
else:
    xmpp = None

summer.download_set_default (tmp_dir = config.get ('tmp_dir'), save_dir = config.get ('save_dir'))
if options.no_cache:
    cache = None
else:
    cache = config.get ('tmp_dir')
summer.feed_set_default (cache_dir = cache, frequency = config.get ('frequency', 0))

if config.get ('torrent_min_port') and config.get ('torrent_max_port'):
    summer.torrent_set_default (min_port = config['torrent_min_port'], max_port = config['torrent_max_port'])

def on_download_complete (dl, save_path, item):
    if xmpp:
        xmpp.send ("Downloaded", 'Saved %s to %s' % (item.get_title (), save_path))
    else:
        print 'Saved %s to %s' % (item.get_title (), save_path)

def on_new_entries (feed, subscription):
    items = feed.get_items ()
    for item in items:
        if xmpp:
            xmpp.send ('Downloading', "Downloading %s" % item.get_title ())
        else:
            print "Downloading %s" % item.get_title ()
        dl = summer.create_download (item)
        if not dl:
            continue
        if subscription.get ('save_dir'):
            save_dir = subscription.get ('save_dir')
        else:
            save_dir = os.path.join (config.get ('save_dir'), subscription.get ('name'))
        dl.set_save_dir (save_dir)
        dl.connect ("download-complete", on_download_complete, item)
        dl.start ()

def main ():
    for subscription in config.get ('subscriptions', []):
        feed = summer.Feed ()
        feed.connect ("new-entries", on_new_entries, subscription)
        feed.start (subscription['url'])

    loop = glib.MainLoop ()
    try:
        loop.run ()
    finally:
        print "\nShutting down... Please wait."
        summer.shutdown ()
        return

if __name__ == "__main__":
    main ()
