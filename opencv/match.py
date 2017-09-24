# -*- coding: utf-8 -*-
"""
Created on Mon Jan 13 20:57:38 2014
@author: duan
"""
import cv2
import numpy as np
img1 = cv2.imread('star2.png',0)
img2 = cv2.imread('star4.png',0)
ret, thresh = cv2.threshold(img1, 127, 255,0)
ret, thresh2 = cv2.threshold(img2, 127, 255,0)
contours,hierarchy = cv2.findContours(thresh,2,1)
cnt1 = contours[0]
contours,hierarchy = cv2.findContours(thresh2,2,1)
cnt2 = contours[0]
ret = cv2.matchShapes(cnt1,cnt2,1,0.0) 
print ret
