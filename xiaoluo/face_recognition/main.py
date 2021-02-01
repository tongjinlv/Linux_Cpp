import face_recognition
import cv2
import sys
import socket
import time,os
from threading import Thread
import RPi.GPIO as GPIO
import serial
import numpy as np
import time,datetime
import Adafruit_PCA9685
identity=0
savephoto=0
count=0
minute=99
cap=cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))

pwm = Adafruit_PCA9685.PCA9685()
servo_min = 150  # Min pulse length out of 4096
servo_max = 600  # Max pulse length out of 4096

def set_servo_pulse(channel, pulse):
    pulse_length = 1000000    # 1,000,000 us per second
    pulse_length //= 60       # 60 Hz
    print('{0}us per period'.format(pulse_length))
    pulse_length //= 4096     # 12 bits of resolution
    print('{0}us per bit'.format(pulse_length))
    pulse *= 1000
    pulse //= pulse_length
    pwm.set_pwm(channel, 0, pulse)

pwm.set_pwm_freq(60)
old_en=0
def show_face(page):
        os.system("sudo chmod 777 /dev/ttyAMA0")
        ser = serial.Serial("/dev/ttyAMA0", 9600)  # 位置1
        ser.flushInput()  
        msg="@GUIS "+str(page)+"\r"
        print(msg)
        ser.write(msg.encode("utf-8"))  # 位置3
        time.sleep(0.1) 
def stack_head(en):
    global old_en
    if(en==old_en):
        return
    old_en=en
    if(en==1):
      for i in range(20,80):#脑袋
        set_angle(13,i)
        time.sleep(0.01) 
      for i in range(150,40,-1):
            set_angle(0,i)
            time.sleep(0.01)
      for i in range(30,100):
            set_angle(1,i)
            time.sleep(0.01)
      show_face(0)
    if(en==2):
       for i in range(80,20,-1):#脑袋
            set_angle(13,i)
            time.sleep(0.01)
       for i in range(40,150):#小兔子1
            set_angle(0,i)
            time.sleep(0.01)
       for i in range(100,30,-1):
            set_angle(1,i)
            time.sleep(0.01)
       show_face(1)
    


def set_angle(channel,angle):
    i=round(angle*3.1+160)
    pwm.set_pwm(channel,0,i)
def sharkear():
    for z in range(0,10):
        for i in range(0,20):
            set_angle(14,i)
            set_angle(15,20-i)
        for i in range(20,0,-1):
            set_angle(14,i)
            set_angle(15,20-i)
known_face_encoding =[]
dirs = os.listdir("/home/pi/work/Linux_Cpp/xiaoluo/face_recognition/known")
for dir_name in dirs:
    dir_name="/home/pi/work/Linux_Cpp/xiaoluo/face_recognition/known/"+dir_name
    print("load face:"+dir_name)
    picture_of_me = face_recognition.load_image_file(dir_name)
    if(len(known_face_encoding)==0):
        known_face_encoding=[face_recognition.face_encodings(picture_of_me)[0]]
    else:
        known_face_encoding +=[face_recognition.face_encodings(picture_of_me)[0]]
def set_pin(pin,value):
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, value)
def chat(conn):
    global savephoto
    msg = conn.recv(1024).decode('utf-8')
    if(msg.find("get identity")>=0):
        conn.send(str(identity).encode())
        print(msg)
    elif(msg.find("save photo")>=0):
        conn.send("ok".encode())
        savephoto=1
        print(msg)
    elif(msg.find("sw page")>=0):
        os.system("sudo chmod 777 /dev/ttyAMA0")
        ser = serial.Serial("/dev/ttyAMA0", 9600)  # 位置1
        ser.flushInput()  
        number = msg.split(":")[1]
        msg="@GUIS "+str(number)+"\r"
        print(msg)
        ser.write(msg.encode("utf-8"))  # 位置3
        time.sleep(0.1) 
        conn.send("ok".encode())
    elif(msg.find("stack head:2")>=0):
        stack_head(2)
        conn.send("ok".encode())
    elif(msg.find("stack head:1")>=0):
        stack_head(1)
        conn.send("ok".encode())
    elif(msg.find("sharkear")>=0):
        sharkear() 
        conn.send("ok".encode())
    else:
        print("not defind:"+msg)
    conn.close()

def tcp_server(tags):
    sk = socket.socket(socket.AF_INET,socket.SOCK_STREAM)  
    sk.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  
    sk.bind(('0.0.0.0',8080))
    sk.listen()
    while True:
        conn,addr = sk.accept()
        Thread(target=chat, args =(conn,)).start()
        print("addr",addr)
        time.sleep(1)
    sk.close()


Thread(target=tcp_server, args =("conn",)).start()

def if_radish(frame):
    h=5
    s=129
    v=86
    lower_hsv = np.array([h,s,v])
    upper_hsv = np.array([h+55,s+100,v+100])
    mask = cv2.inRange(frame, lower_hsv, upper_hsv)
    kernel = np.ones((5,5),np.uint8)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
    contours, hierarchy = cv2.findContours(mask,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)  
    cv2.drawContours(mask,contours,-1,(0,0,255),3) 
    count=len(contours)
    for i in range(0,count):
        size=cv2.contourArea(contours[i])
        print(i,size)
        if(size>200):
            return True
    return False
def led_show(a,b):
    for i in range(0,a):
        time.sleep(b)
        set_pin(40,True)
        time.sleep(b)
        set_pin(40,False)
        ret,frame = cap.read()
while True:
    ret,frame = cap.read()
    frame=cv2.resize(frame,(320,240))
    count+=1
    if(count==10):
        count=1
        face_locations = face_recognition.face_locations(frame)
        print("faces count:"+str(len(face_locations)))
        if(len(face_locations)>0):
            if(savephoto==1):
                name="/home/pi/work/Linux_Cpp/xiaoluo/face_recognition/known/"+time.strftime("%Y%m%d%H%M%S", time.localtime()) +".jpg"
                cv2.imwrite(name,frame)
                print("write "+name)
                picture_of_me = face_recognition.load_image_file(name)
                if(len(known_face_encoding)==0):
                    known_face_encoding=[face_recognition.face_encodings(picture_of_me)[0]]
                else:
                    known_face_encoding +=[face_recognition.face_encodings(picture_of_me)[0]]
                savephoto=0
                dirs = os.listdir("/home/pi/work/Linux_Cpp/xiaoluo/face_recognition/known")
            unknown_face_encoding = face_recognition.face_encodings(frame,face_locations)
            if(len(unknown_face_encoding)>0):
                find_count=0
                for num in range(0,len(known_face_encoding)):
                    results = face_recognition.compare_faces([known_face_encoding[num]], unknown_face_encoding[0],tolerance=0.4)
                    if results[0] == True:
                        print("It's a picture of "+dirs[num])
                        identity=1
                        set_pin(40,True)
                        find_count+=1
                if(find_count==0):
                    if(if_radish(frame)):
                            if(datetime.datetime.now().minute!=minute):
                                minute=datetime.datetime.now().minute
                                led_show(10,0.01)
                                name="/home/pi/work/Linux_Cpp/xiaoluo/face_recognition/known/"+time.strftime("%Y%m%d%H%M", time.localtime()) +".jpg"
                                cv2.imwrite(name,frame)
                                print("write "+name)
                                picture_of_me = face_recognition.load_image_file(name)
                                if(len(known_face_encoding)==0):
                                    known_face_encoding=[face_recognition.face_encodings(picture_of_me)[0]]
                                else:
                                    known_face_encoding +=[face_recognition.face_encodings(picture_of_me)[0]]
                                savephoto=0
                                dirs = os.listdir("/home/pi/work/Linux_Cpp/xiaoluo/face_recognition/known")
                            else:
                                print("Too often。。。。")
            else:
                print("又没找到")
        else:
            identity=0
            set_pin(40,False)


