# -*- coding:utf-8 -*-
__author__ = 'Microcosm'

import cv2
import numpy as np
from matplotlib import pyplot as plt

img = cv2.imread("sn.jpg",0)
img2 = img.copy()
template = cv2.imread("hand.png",0)
w,h = template.shape[::-1]

# 6 中匹配效果对比算法
methods = ['cv2.TM_CCOEFF', 'cv2.TM_CCOEFF_NORMED', 'cv2.TM_CCORR',
           'cv2.TM_CCORR_NORMED', 'cv2.TM_SQDIFF', 'cv2.TM_SQDIFF_NORMED']
img = img2.copy()
method = eval('cv2.TM_CCOEFF_NORMED')
res = cv2.matchTemplate(img,template,method)
min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)
print (min_val)
if method in [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED]:
	top_left = min_loc
else:	
	top_left = max_loc
bottom_right = (top_left[0] + w, top_left[1] + h)
cv2.rectangle(img,top_left, bottom_right, 255, 2)
cv2.imwrite("pp.jpg",img)

