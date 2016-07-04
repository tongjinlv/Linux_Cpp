# -*- coding: UTF-8 -*-
#!/usr/bin/env python
import socket
import os
import time 
ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
ss.connect(('112.74.98.39',1500))
ss.sendall('H718L9X19767N1G2')
data=ss.recv(1024)
print "revice %s"%data
for i in range(1, 50):
	os.system('sleep 1')
	sdata='{"sensorDatas":[{"value":'+str(i)+'},{"value":44},{"value":4.20},{"value":23}]}\n'
	ss.sendall(sdata)
	print sdata
	revice=ss.recv(1024)
        print "revice %s"%revice

else:
    	print('发送完毕')
	ss.close()
