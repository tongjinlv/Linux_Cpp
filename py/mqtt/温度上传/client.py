# -*- coding: UTF-8 -*-
#!/usr/bin/env python
import socket
import os
import time 
import commands
def get_cpu_temp():
    	 tempFile = open( "/sys/class/thermal/thermal_zone0/temp" )
     	 cpu_temp = tempFile.read()
    	 tempFile.close()
    	 return float(cpu_temp)/1000

def get_gpu_temp():
    gpu_temp = commands.getoutput( '/opt/vc/bin/vcgencmd measure_temp' ).replace( 'temp=', '' ).replace( '\'C', '' )
    return  float(gpu_temp)
ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
ss.connect(('112.74.98.39',1500))
ss.sendall('H718L9X19767N1G2')
data=ss.recv(1024)
print "revice %s"%data
while(True):
	os.system('sleep 1')	
	sdata='{"sensorDatas":[{"value":'+str(get_cpu_temp())+'},{"value":'+str(get_gpu_temp())+'},{"value":4.20},{"value":23}]}\n'
	ss.sendall(sdata)
	print sdata
	revice=ss.recv(1024)
        print "revice %s"%revice
    	print('发送完毕')
