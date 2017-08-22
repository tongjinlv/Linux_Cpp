import cv2

ascii_char = list("$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,\"^`'. ")

def get_char(gray_number):
    length = len(ascii_char)
    unit = (256.0 + 1)/length
    return ascii_char[int(gray_number/unit)]

if __name__ == '__main__':
    image1 = cv2.imread('1.png')
    image = cv2.resize(image1,(190,60))
    gray = cv2.cvtColor(image,cv2.COLOR_BGR2GRAY)
    txt = ""
    for i in range(image.shape[0]):
        for j in range(image.shape[1]):
            txt += get_char(gray[i,j])
        txt += '\n'
    print txt
    f = open('output.txt','w')
    f.write(txt)
