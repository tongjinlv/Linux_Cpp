#encoding=utf-8
import cv2
import time
from poster.encode import multipart_encode
import urllib2
import os,shutil
import sys
from urllib2 import Request, urlopen, URLError, HTTPError
from poster.encode import multipart_encode
from poster.streaminghttp import register_openers
import datetime
import socket
import time

DATAMARK=1
TLINK=2
GIZWITS=3
print "open video0"
cap=cv2.VideoCapture(0)
print cap
pre_frame = None
def post(file):
    register_openers()
    f=open(file, "rb")
    datagen, headers = multipart_encode({"myFile": f})
    request = urllib2.Request("http://trtos.com/web/datamark/upload.php", datagen, headers)
    try:
        response = urllib2.urlopen(request)
        print response.read()
         
    except URLError,e:
        print e.reason
        print e.code




def send(context,name,id):
   if name==DATAMARK:
       context=context+"\n"
       ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
       ss.connect(('trtos.com',80))
       msg="POST /php/api.php?id="+id+" HTTP/1.1\n"+ \
            "Content-Type: application/x-www-form-urlencoded\n"+ \
            "Host: trtos.com\n"+ \
            "Content-Length: "+str(len(context))+"\n"+ \
            "Expect: 100-continue\n"+ \
            "Connection: Keep-Alive\n\n\n"+context
       ss.sendall(msg)
       ss.close()
       print msg
   elif name==TLINK:
       ss=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
       ss.connect(('tcp.tlink.io',8647))
       time.sleep(1)
       ss.sendall(id)
       time.sleep(1)
       ss.sendall(context)
       ss.close()
       print id,context,name
   elif name==GIZWITS:
       print "机智云"
      
while(1):
    ret,frame=cap.read()
    curframe=frame.copy()
    k=cv2.waitKey(1)
    cv2.imwrite("raw.jpg",frame)
    gray_img=cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray_img=cv2.resize(gray_img, (500, 500))
    gray_img=cv2.GaussianBlur(gray_img, (21, 21), 0)
    if pre_frame is None:
        pre_frame = gray_img
    else:
        img_delta = cv2.absdiff(pre_frame, gray_img)
        thresh = cv2.threshold(img_delta, 25, 255, cv2.THRESH_BINARY)[1]
        thresh = cv2.dilate(thresh, None, iterations=2)
        thresh,contours, hierarchy = cv2.findContours(thresh.copy(),cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
        for c in contours:
            if cv2.contourArea(c) < 3000:
                 print cv2.contourArea(c)
                 continue
            else:
                 (x,y,w,h) = cv2.boundingRect(c)
                 cv2.rectangle(curframe,(x,y),(x+w,y+h),(0,255,0),2)
                 cv2.imwrite("4UTmPcpw.jpg",curframe)
                 post("4UTmPcpw.jpg")
                 shutil.move("4UTmPcpw.jpg","/home/pi/record/"+datetime.datetime.now().strftime('%Y_%m_%d_%H_%M_%S')+".jpg")
                 send("area:"+str(cv2.contourArea(c)),DATAMARK,"4UTmPcpw")
    pre_frame = gray_img
    time.sleep(1)
cap.release()
