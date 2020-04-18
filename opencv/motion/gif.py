import cv2
import imageio
img1=cv2.imread('./curframe.jpg',1)
img2=cv2.imread("./raw.jpg",1)
img1 = cv2.resize(img1, (360,460))
img2 = cv2.resize(img2, (360,460))
print(img1.shape[:2])
print(img2.shape[:2])
buff=[]
buff.append(img1)
buff.append(img2)
gif=imageio.mimsave('yang.gif',buff,'GIF',duration=0.1)
print gif