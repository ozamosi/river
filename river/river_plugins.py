#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import imp
import xdg.BaseDirectory as bd

class Plugins:
    plugins = {}

    def __init__ (self, config = None):
        if config:
            self.load_plugins (config)

    def new_entry (self, dl, item):
        for p in self.plugins.keys ():
            self.plugins[p]['new_entry'] (self.plugins[p]['data'], dl, item)

    def download_started (self, dl, item):
        for p in self.plugins.keys ():
            self.plugins[p]['download_started'] (self.plugins[p]['data'], dl, item)

    def download_complete (self, dl, item):
        for p in self.plugins.keys ():
            self.plugins[p]['download_complete'] (self.plugins[p]['data'], dl, item)

    def download_update (self, dl, downloaded, length, item):
        for p in self.plugins.keys ():
            self.plugins[p]['download_update'] (self.plugins[p]['data'], dl, downloaded, length, item)


    def load_plugins (self, config):
        for p in config:
            plugin_paths = list (bd.load_data_paths ('river', p['name'] + '.py'))
            if not plugin_paths:
                continue

            m = imp.load_source('river-plugins-' + p['name'], plugin_paths[0])

            self.plugins[p['name']] = {}

            self.plugins[p['name']]['new_entry'] =         m.new_entry
            self.plugins[p['name']]['download_started'] =  m.download_started
            self.plugins[p['name']]['download_complete'] = m.download_complete
            self.plugins[p['name']]['download_update'] =   m.download_update
            self.plugins[p['name']]['shutdown'] =          m.shutdown

            self.plugins[p['name']]['data'] = {'config': p['config']}

            m.init (self.plugins[p['name']]['data'])

    def shutdown (self):
        for p in self.plugins.keys ():
            self.plugins[p]['shutdown'] (self.plugins[p]['data'])
