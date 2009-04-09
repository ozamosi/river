#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import summer

def flatten_item(i):
    return {'author':        i.get_author(), 
            'description':   i.get_description(), 
            'downloadables': [flatten_downloadable(x) for x in i.get_downloadables()],
            'id':            i.get_id(),
            'title':         i.get_title(),
            'updated':       i.get_updated(),
            'web_url':       i.get_web_url()}

def flatten_downloadable(d):
    return {'url': d.get_url(), 'mime': d.get_mime(), 'length': d.get_length()}
