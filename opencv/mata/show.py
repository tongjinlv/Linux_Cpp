import cv2


cameraCapture = cv2.VideoCapture(0)
cv2.namedWindow('MyWindow')

success, frame = cameraCapture.read()
while success and cv2.waitKey(1) == 255:
    cv2.imshow('MyWindow',frame)
    success, frame = cameraCapture.read()

cameraCapture.release()
