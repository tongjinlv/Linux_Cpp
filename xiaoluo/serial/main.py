# -*- coding:utf-8 -*-
import serial
import time
ser = serial.Serial("/dev/ttyAMA0", 9600)  # 位置1
ser.flushInput()  # 位置2
ser.write("@GUIS 1\r".encode("utf-8"))  # 位置3
time.sleep(0.1)  # 位置8
