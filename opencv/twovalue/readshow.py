# -*- coding: utf-8 -*-
import cv2
 
# 读图片
image=cv2.imread('1.jpg', cv2.CV_LOAD_IMAGE_COLOR)#Load the image
#Or just: image=cv.LoadImage('img/image.png')
img1=image.copy() 
img2=cv2.cvtColor(img1,cv2.COLOR_BGR2GRAY) 
#huidu
# 写图片
cv2.imwrite("img1.jpg",img1,[int(cv2.IMWRITE_JPEG_QUALITY), 5]);
cv2.imwrite("img2.jpg",img2)
print(repr(img1[0,0,0]))
print(repr(img2[0,0]))
ret,img3=cv2.threshold(img2,127,255,cv2.THRESH_TOZERO)
cv2.imwrite("thresh1.jpg",img3)
