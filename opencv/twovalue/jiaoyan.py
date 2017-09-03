import cv2  
import numpy as np  
  
def salt(img, n):  
    for k in range(n):  
        i = int(np.random.random() * img.shape[1]);  
        j = int(np.random.random() * img.shape[0]);  
        if img.ndim == 2:   
            img[j,i] = 255  
        elif img.ndim == 3:   
            img[j,i,0]= 255  
            img[j,i,1]= 255  
            img[j,i,2]= 255  
    return img  
  
if __name__ == '__main__':  
	img = cv2.imread("1.jpg")  
	simg = salt(img, 500)
	r=cv2.split(simg)[0]
	g=cv2.split(simg)[1]
	b=cv2.split(simg)[2]
	merged = cv2.merge([b,g,r])
	cv2.imwrite("img1.jpg",merged)
	ret,thresh1=cv2.threshold(simg,127,255,cv2.THRESH_BINARY)
	cv2.imwrite("thresh1.jpg",thresh1) 
