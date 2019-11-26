import cv2
import time

cap=cv2.VideoCapture(0)

pre_frame = None
while(1):
    ret,frame=cap.read()
    k=cv2.waitKey(1)
    cv2.imwrite("raw.jpg",frame)
    gray_img=cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray_img=cv2.resize(gray_img, (500, 500))
    gray_img=cv2.GaussianBlur(gray_img, (21, 21), 0)
    if pre_frame is None:
        pre_frame = gray_img
    else:
        img_delta = cv2.absdiff(pre_frame, gray_img)
        thresh = cv2.threshold(img_delta, 25, 255, cv2.THRESH_BINARY)[1]
        thresh = cv2.dilate(thresh, None, iterations=2)
        thresh,contours, hierarchy = cv2.findContours(thresh.copy(),cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
        for c in contours:
            if cv2.contourArea(c) < 1000:
                 print cv2.contourArea(c)
                 continue
            else:
                 (x,y,w,h) = cv2.boundingRect(c)
                 cv2.rectangle(frame,(x,y),(x+w,y+h),(0,255,0),2)
                 cv2.imwrite("frame.jpg",gray_img)
                 print "b"
                 print cv2.contourArea(c)
    pre_frame = gray_img
    time.sleep(1)
cap.release()
