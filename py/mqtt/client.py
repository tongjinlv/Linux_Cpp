# -*- coding: UTF-8 -*-
#!/usr/bin/env python
import socket
import os
import time 
ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
ss.connect(('112.74.98.39',1500))
#ss.connect(('192.168.1.11',8080))
ss.sendall('H718L9X19767N1G2')
data=ss.recv(1024)
print "server dafu %s"%data
os.system('sleep 1')
ss.sendall('{"sensorDatas":[{"value":13.43},{"value":44}]}\n')
ss.close()
