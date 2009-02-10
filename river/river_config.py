#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Example configuration file:
#
# <river>
#  <save_dir>/home/username/mediafiles/</save_dir>
#  <tmp_dir>/home/username/.cache/</tmp_dir>
#  <frequency>900</frequency>
#  <torrent_min_port>6881</torrent_min_port>
#  <torrent_max_port>6889</torrent_max_port>
#  <subscription>
#   <name>Awesome feed</name>
#   <url>http://awesome.com/feed</url>
#   <save_dir>(optional)/home/username/other_mediafiles_dir</save_dir>
#  </subscription>
# </river>

from xml.etree import ElementTree
from xdg import BaseDirectory
import os.path

class Config:
    result = {}
    def __init__ (self, file = None):
        config = ElementTree.ElementTree ()
        if not file:
            file = os.path.join (BaseDirectory.save_config_path ("river"), "config.xml")
        config.parse (file)
        root = config.getroot ()
        for element in root.getchildren ():
            if element.tag == 'save_dir':
                self.result['save_dir'] = element.text
            elif element.tag == 'tmp_dir':
                self.result['tmp_dir'] = element.text
            elif element.tag == 'frequency':
                self.result['frequency'] = int (element.text)
            elif element.tag == 'torrent_min_port':
                self.result['torrent_min_port'] = int (element.text)
            elif element.tag == 'torrent_max_port':
                self.result['torrent_max_port'] = int (element.text)
            elif element.tag == 'subscription':
                subscription = {}
                for selement in element.getchildren ():
                    if selement.tag == 'name':
                        subscription['name'] = selement.text
                    if selement.tag == 'url':
                        subscription['url'] = selement.text
                    if selement.tag == 'save_dir':
                        subscription['save_dir'] = selement.text
                if not self.result.get ('subscriptions'):
                    self.result['subscriptions'] = [subscription]
                else:
                    self.result['subscriptions'].append (subscription)
