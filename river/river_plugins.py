#!/usr/bin/env python
# -*- encoding: utf-8 -*-

class Plugins:
    cb_new_entries = []
    cb_download_complete = []

    p_config = {}

    def __init__(self):
        pass

    def new_entries(self):
        pass

    def download_complete(self):
        pass

    def register_plugin(self, name, path, config):
        pass
