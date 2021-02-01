import cv2,sys



cap=cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))

while True:
    ret,frame = cap.read()
    frame=cv2.resize(frame,(320,240))
    if(cv2.waitKey(1)>-1):
        cv2.imwrite("cap.jpg",frame)
        cv2.destroyAllWindows()
        sys.exit()
    cv2.imshow('cap',frame)
    