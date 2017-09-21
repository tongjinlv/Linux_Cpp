import cv2
import time

cap = cv2.VideoCapture(0)
print time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
cap.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH,200)
print time.strftime('B%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
cap.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT,100)
print time.strftime('C%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
ret, frame = cap.read()
print time.strftime('D%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
cv2.imwrite("image.jpg",frame)
print time.strftime('E%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
cap.release()
