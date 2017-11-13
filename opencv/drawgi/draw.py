#!/usr/bin/python
# -*- coding: UTF-8 -*-
import numpy as np
import sys
import cv2

print str(sys.argv)
img=np.zeros((512,512,3), np.uint8)
cv2.line(img,(20,20),(80,20),(255,255,255),1)
cv2.line(img,(20,21),(80,21),(255,0,255),1)
cv2.imwrite("line.jpg",img);






