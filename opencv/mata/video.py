import cv2  
import numpy
cap = cv2.VideoCapture("a.mp4")
while(1):
    # get a frame
    ret, frame = cap.read()
    # show a frame
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gradX = cv2.Sobel(gray, ddepth=cv2.cv.CV_32F, dx=1, dy=0, ksize=-1)
    gradY = cv2.Sobel(gray, ddepth=cv2.cv.CV_32F, dx=0, dy=1, ksize=-1)
    # subtract the y-gradient from the x-gradient
    gradient = cv2.subtract(gradX, gradY) 
    gradient = cv2.convertScaleAbs(gradient)
    ret, binary = cv2.threshold(gradient,127,255,cv2.THRESH_BINARY)  
    contours, hierarchy = cv2.findContours(binary,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)  
    cv2.drawContours(gradient,contours,-1,(0,0,255),3)  
    cv2.imshow("capture", gradient)
    if cv2.waitKey(100) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows() 
