#!/usr/bin/env python
# -*- coding: utf-8 -*-

def new_entry (pdata, dl, item):
    print 'Added download of %s' % item['title']

def download_complete (pdata, dl, item):
    print 'Saved %s to %s' % (item['title'], dl.get_save_path ())

def download_started (pdata, dl, item):
    print 'Download of %s started' % item['title']

def download_update (pdata, dl, downloaded, length, item):
    print 'Status update: %s. Downloaded: %s/%s' % (item['title'], str(downloaded), str(length))

def download_error (pdata, dl, error, item):
    print 'Error in %s: %s'% (item['title'], error.message)

def init (pdata):
    print 'Loaded simple print plugin'

def shutdown (pdata):
    print 'Unloaded simple print plugin'
