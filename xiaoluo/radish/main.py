import cv2,sys
import numpy as np


cap=cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))

h=5
s=129
v=86
key=None
while True:
    ret,frame = cap.read()
    frame=cv2.resize(frame,(640,480))
    key=cv2.waitKey(1)
    if(key>-1):
        print(key)
        if(key==27):
            cv2.imwrite("cap.jpg",frame)
            cv2.destroyAllWindows()
            sys.exit()
        elif(key==180):
            h+=1
        elif(key==177):
            h-=1
        elif(key==181):
            s+=1
        elif(key==178):
            s-=1
        elif(key==182):
            v+=1
        elif(key==179):
            v-=1
        print("h,s,v",h,s,v)
    lower_hsv = np.array([h,s,v])
    upper_hsv = np.array([h+55,s+100,v+100])
    mask = cv2.inRange(frame, lower_hsv, upper_hsv)
    cv2.imshow('cap',frame)
    kernel = np.ones((5,5),np.uint8)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
    contours, hierarchy = cv2.findContours(mask,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)  
    cv2.drawContours(mask,contours,-1,(0,0,255),3) 
    count=len(contours)
    for i in range(0,count):
        size=cv2.contourArea(contours[i])
        print(i,size)
        if(size>1000):
            cv2.destroyAllWindows()
            sys.exit()
    cv2.imshow('mask',mask)
    