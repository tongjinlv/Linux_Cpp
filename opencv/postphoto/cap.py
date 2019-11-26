#!usr/bin/python
# image.py
# -*- coding=utf-8 -*-
import cv2
import time

cap=cv2.VideoCapture(0)
print "waiting cap raw"
time.sleep(3)
ret,frame=cap.read()
cv2.imwrite("raw.jpg",frame)
print ret
cap.release()
