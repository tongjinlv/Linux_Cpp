#encoding=utf-8
import cv2
import time

frame1=cv2.imread("raw.jpg");
frame2=cv2.imread("raw1.jpg");

frame1=cv2.cvtColor(frame1, cv2.COLOR_BGR2GRAY)
frame2=cv2.cvtColor(frame2, cv2.COLOR_BGR2GRAY)
cv2.imwrite("raw1-1.jpg",frame1)
cv2.imwrite("raw2-1.jpg",frame2)
frame1=cv2.GaussianBlur(frame1, (21, 21), 0)
frame2=cv2.GaussianBlur(frame2, (21, 21), 0)
cv2.imwrite("raw1-2.jpg",frame1)
cv2.imwrite("raw2-2.jpg",frame2)

img_delta = cv2.absdiff(frame1, frame2)
cv2.imwrite("img_delta.jpg",img_delta)
thresh = cv2.threshold(img_delta, 25, 255, cv2.THRESH_BINARY)[1]
cv2.imwrite("thresh.jpg",thresh)
thresh = cv2.dilate(thresh, None, iterations=2)
cv2.imwrite("thresh1.jpg",thresh)
thresh,contours, hierarchy = cv2.findContours(thresh.copy(),cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
raw=cv2.imread("raw.jpg");
for c in contours:
   print cv2.contourArea(c)
   if cv2.contourArea(c)>1000 :
     (x,y,w,h) = cv2.boundingRect(c)
     cv2.rectangle(raw,(x,y),(x+w,y+h),(0,255,0),2)
cv2.imwrite("raw_s.jpg",raw)
exit()
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
