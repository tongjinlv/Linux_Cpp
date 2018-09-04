# -*- coding: UTF-8 -*-
#!/usr/bin/env python
import socket
import os
import time 
import commands
def get_cpu_temp():
    	 return float(1.33)

def get_gpu_temp():
    return  float(9.13)
ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
print "connect..."
ss.connect(('tcp.tlink.io',8647))
print "connect..ok"
ss.sendall('H718L9X19767N1G2')
print "send id ok"
#data=ss.recv(1)
#print "revice %s"%data
while(True):
	os.system('sleep 1')	
	#sdata='{"sensorDatas":[{"value":'+str(get_cpu_temp())+'},{"value":'+str(get_gpu_temp())+'},{"value":4.20},{"value":23}]}\n'
        sdata='#13.3,55#'
	ss.sendall(sdata)
	print sdata
    	print('发送完毕')
