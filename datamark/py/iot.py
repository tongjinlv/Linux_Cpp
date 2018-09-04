# -*- coding: UTF-8 -*-
#!/usr/bin/env python
import socket
import time

DATAMARK=1
TLINK=2
GIZWITS=3


def send(context,name,id):
    
   if name==DATAMARK:
       context=context+"\n"
       ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
       ss.connect(('trtos.com',80))
       msg="POST /php/api.php?id="+id+" HTTP/1.1\n"+ \
            "Content-Type: application/x-www-form-urlencoded\n"+ \
            "Host: trtos.com\n"+ \
            "Content-Length: "+str(len(context))+"\n"+ \
            "Expect: 100-continue\n"+ \
            "Connection: Keep-Alive\n\n\n"+context
       ss.sendall(msg)
       ss.close()
       print msg
   elif name==TLINK:
       ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
       ss.connect(('tcp.tlink.io',8647))
       time.sleep(1)
       ss.sendall(id)
       time.sleep(1)
       ss.sendall(context)
       ss.close()
       print id,context,name
   elif name==GIZWITS:
       print "机智云"
      
