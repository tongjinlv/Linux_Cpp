import numpy as np
import argparse
import cv2
image = cv2.imread('1.jpg')
color = [
    ([0, 70, 70], [100, 255, 255])
]
 
for (lower, upper) in color:
    lower = np.array(lower, dtype = "uint8")
    upper = np.array(upper, dtype = "uint8")
    mask = cv2.inRange(image, lower, upper)
    output = cv2.bitwise_and(image, image, mask = mask)
    cv2.imwrite("2.jpg", np.hstack([image, output]))
    cv2.waitKey(0)