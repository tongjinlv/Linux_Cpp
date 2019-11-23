import cv2
import time

cap=cv2.VideoCapture(0)
print "waiting cap raw"
time.sleep(3)
ret,frame=cap.read()
cv2.imwrite("raw.jpg",frame)
print ret


print "waiting cap raw"
time.sleep(3)
ret,frame=cap.read()
cv2.imwrite("raw1.jpg",frame)
print ret

cap.release()
