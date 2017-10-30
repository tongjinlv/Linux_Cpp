import cv2
cap = cv2.VideoCapture(0)
#cap.set(cv2.CAP_PROP_MODE,3)
#print cap.get(cv2.CAP_PROP_MODE)  
cap.open(0)
ret, frame = cap.read()
cv2.imwrite("fangjian2.jpeg", frame)
cap.release()
