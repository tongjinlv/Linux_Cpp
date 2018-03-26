import cv2
import cv2.cv as cv
import numpy as np

img1=cv2.imread("img/1.jpg");
img2 = cv2.cvtColor(img1, cv2.COLOR_BGR2HSV)
cv2.imwrite("img/2.jpg",img2);
H, S, V = cv2.split(img2)
LowerBlue = np.array([0, 100, 0])
UpperBlue = np.array([255, 255, 255])
mask = cv2.inRange(img2, LowerBlue, UpperBlue)
img3 = cv2.bitwise_and(img1, img1, mask=mask)
cv2.imwrite("img/3.jpg",img3)
img4=cv2.cvtColor(img3,cv2.COLOR_BGR2GRAY)
cv2.imwrite("img/4.jpg",img4)
ret,img5=cv2.threshold(img4,10,255,cv2.THRESH_BINARY)
cv2.imwrite("img/5.jpg",img5)
kernel=np.uint8(np.zeros((5,5)))
for x in range(5):
    kernel[x,2]=1;
    kernel[2,x]=1;
img6 = cv2.dilate(img5,kernel)
cv2.imwrite("img/6.jpg",img6)
img7 = cv2.absdiff(img5,img6)
cv2.imwrite("img/7.jpg",img7)
circles= cv2.HoughCircles(img6,cv.CV_HOUGH_GRADIENT,1,40,param1=40,param2=30,minRadius=100,maxRadius=250)
print(circles)
print(len(circles[0]))
for circle in circles[0]:
    print(circle[2])
    x=int(circle[0])
    y=int(circle[1])
    r=int(circle[2])
    cv2.circle(img6,(x/2,y/2),r,(255,255,255),-1)
cv2.imwrite("img/8.jpg",img6)
