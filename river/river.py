#!/usr/bin/env python
# -*- encoding: utf-8 -*-
import summer
import river_config
import sys
import os.path
from optparse import OptionParser
try:
    import glib
except ImportError:
    import gobject as glib

import river_plugins

parser = OptionParser ()
parser.add_option ("--no-cache", dest = "no_cache", help = "Disable the cache file, forcing River to download everything it sees", action = "store_true", default = False)
parser.add_option ("-c", "--config", dest = "config", help = "Use a custom config file instead the default", default = None)

(options, args) = parser.parse_args ()

config = river_config.Config (options.config).result


summer.download_set_default (tmp_dir = config.get ('tmp_dir'), save_dir = config.get ('save_dir'))
if options.no_cache:
    cache = None
else:
    cache = config.get ('tmp_dir')
summer.feed_set_default (cache_dir = cache, frequency = config.get ('frequency', 0))

if config.get ('torrent_min_port') and config.get ('torrent_max_port'):
    summer.torrent_set_default (min_port = config['torrent_min_port'], max_port = config['torrent_max_port'])

plugins = river_plugins.Plugins(config['plugins'])

def on_download_complete (dl, item):
    plugins.download_complete (dl, item)

def on_download_started (dl, item):
    plugins.download_started (dl, item)

def on_download_update (dl, downloaded, length, item):
    plugins.download_update (dl, downloaded, length, item)

def on_download_error (dl, error, item):
    plugins.download_error (dl, error, item)

def on_new_entries (feed, subscription):
    items = feed.get_items ()
    for item in items:
        dl = summer.create_download (item)
        if not dl:
            continue
        if subscription.get ('save_dir'):
            save_dir = subscription.get ('save_dir')
        else:
            save_dir = os.path.join (config.get ('save_dir'), subscription.get ('name'))
        dl.set_save_dir (save_dir)
        dl.connect ("download-complete", on_download_complete, item)
        dl.connect ("download-started", on_download_started, item) 
        dl.connect ("download-update", on_download_update, item)
        dl.on_error (on_download_error, item)
        dl.start ()

        plugins.new_entry (dl, item)

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
        plugins.shutdown ()
        summer.shutdown ()
        print "Shutdown complete"
        return

if __name__ == "__main__":
    main ()
