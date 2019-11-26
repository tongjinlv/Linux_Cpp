#!usr/bin/python
# image.py
# -*- coding=utf-8 -*-
from poster.encode import multipart_encode
import urllib2
import sys
from urllib2 import Request, urlopen, URLError, HTTPError
from poster.encode import multipart_encode
from poster.streaminghttp import register_openers
  
register_openers()
f=open(sys.argv[1], "rb")

datagen, headers = multipart_encode({"myFile": f})
request = urllib2.Request("http://trtos.com/web/datamark/upload.php", datagen, headers)
try:
    response = urllib2.urlopen(request)
    print response.read()
     
except URLError,e:
    print e.reason
    print e.code
