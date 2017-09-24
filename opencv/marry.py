# -*- coding: utf-8 -*-
"""
Created on Sat Jan 18 16:33:15 2014
@author: duan
"""
import cv2
import numpy as np
from matplotlib import pyplot as plt
img_rgb = cv2.imread('mario_coin.png')
img_gray = cv2.cvtColor(img_rgb, cv2.COLOR_BGR2GRAY)
temp1 = cv2.imread('mario.png',0)
temp2=cv2.imread('ma.png',0)
w, h = temp2.shape[::-1]
res = cv2.matchTemplate(img_gray,temp2,cv2.TM_CCOEFF_NORMED)
threshold = 0.6
#umpy.where(condition[, x, y])
#Return elements, either from x or y, depending on condition. #If only condition is given, return condition.nonzero().
loc = np.where( res >= threshold)
for pt in zip(*loc[::-1]):
    cv2.rectangle(img_rgb, pt, (pt[0] + w, pt[1] + h), (0,0,255), 2)
cv2.imwrite('res.png',img_rgb)
