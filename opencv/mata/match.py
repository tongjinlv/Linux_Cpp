# -*- coding: utf-8 -*-

import numpy as np
import cv2

# 黑色的图片
img=np.zeros((512,512,3), np.uint8)

# 一条线
cv2.line(img,(0,0),(511,511),(255,0,0),5)
# 画一个矩形（左上角顶点和右下角顶点的坐标）
cv2.rectangle(img,(384,0),(510,128),(0,255,0),3)
# 画一个圆（中心点坐标和半径）
cv2.circle(img,(447,63), 63, (0,0,255), -1)
# 画一个椭圆 （0，180代表半个椭圆,0,360整个）
cv2.ellipse(img,(256,256),(100,50),0,0,360,255,-1)
# 具有四个顶点的多边形
# pts=np.array([[10,5],[20,30],[70,20],[50,10]], np.int32)
# pts=pts.reshape((-1,1,2))

# 添加文字
font=cv2.FONT_HERSHEY_SIMPLEX
cv2.putText(img,'OpenCV',(10,500), font, 4,(255,255,255),2)

#显示图片,最终的绘图结果
cv2.namedWindow('example')
cv2.imshow('example', img)
cv2.waitKey(0)
cv2.destroyWindow(winname)
