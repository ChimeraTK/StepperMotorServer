#!/usr/bin/python3

'''
  Dictionary to shorten too long PV names when using the EPICS IOC Adapter.
  To be used with teh xml2db tool provided with the adater.
'''

def abbreviation():

  abbr = dict([
            ('Translation', 'Transl'),
            ('reference', 'ref')]),
            ('relative", "rel')
            
  
  print "Loading abbreviations, abbr=" + str(abbr)
  return abbr
