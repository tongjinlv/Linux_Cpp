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
ss.connect(('112.74.98.39',8647))
os.system('sleep 1')
ss.sendall('I0F71J0ZW299WZ7O')
data=ss.recv(1024)
print "revice %s"%data
for i in range(1, 50):
	os.system('sleep 1')
	sdata="#1#"
	ss.sendall(sdata)
	print sdata
	revice=ss.recv(1024)
        print "revice %s"%revice

else:
    	print('发送完毕')
	ss.close()
