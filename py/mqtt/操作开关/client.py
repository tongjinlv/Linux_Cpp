# -*- coding: UTF-8 -*-
#!/usr/bin/env python
import socket
import os
import time 
import RPi.GPIO as GPIO

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.OUT)
ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
ss.connect(('112.74.98.39',8647))
os.system('sleep 1')
ss.sendall('I0F71J0ZW299WZ7O')
while(True):
	os.system('sleep 1')
	revice=ss.recv(1024)
        print "revice %s"%revice
	s=revice.find('{"sensorDatas":[{"sensorsId":23703,"switcher":0}]')
	if(int(s)<0):
		print "OFF"
		GPIO.output(21, GPIO.HIGH)
	else:
		print "ON"
		GPIO.output(21, GPIO.LOW)

