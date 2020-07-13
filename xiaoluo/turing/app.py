
import tts
import chardet
import RPi.GPIO as GPIO
import os,sys
import time
GPIO.setmode(GPIO.BOARD)
import socket
def get_identity():
    try:
        sk = socket.socket()
        sk.connect(('127.0.0.1',8080))
        sk.send(b"get identity")
        msg = sk.recv(1024).decode('utf-8')
        print("get_identity",msg)
        sk.close()
        return msg
    except:
        print("人脸识别服务 发生异常")
        return "NULL"
def switch_page(page):
    try:
        sk = socket.socket()
        sk.connect(('127.0.0.1',8080))
        msg="sw page:"+str(page)
        sk.send(msg.encode("utf-8"))
        msg = sk.recv(1024).decode('utf-8')
        print("switch_page",msg)
        sk.close()
        return msg
    except:
        print("人脸识别服务 发生异常")
        return "NULL"
def opendoor(pin,open):
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(pin,GPIO.OUT)
    p=GPIO.PWM(pin,150)
    p.start(0)
    try:
        if(open):
            for dc in range(30,0,-1): 
                p.ChangeDutyCycle(dc)
                time.sleep(0.03)
                print(dc)
        else:
            for dc in range(0,30,1): 
                p.ChangeDutyCycle(dc)
                time.sleep(0.03)
                print(dc)
    except KeyboardInterrupt:
        print("error")
        pass
    p.stop()
    GPIO.cleanup()

def callback(text_):
    tts.tts(text_)
    switch_page(0)
    os.system("mplayer -af volume=10 demo.mp3")
    switch_page(1)
def app(text_):
    print("text_",text_)
    if(text_==""):
        callback("没事 我睡觉去啦，有事呼唤我")
        os.system("python3 /home/pi/work/Linux_Cpp/xiaoluo/snowboy/demo.py /home/pi/work/Linux_Cpp/xiaoluo/snowboy/xiaotuxiaotu.pmdl /home/pi/work/Linux_Cpp/xiaoluo/snowboy/zmkm.pmdl &")
        sys.exit(1)
    if(chardet.detect(str.encode(text_))["encoding"]!="utf-8"):
        print("编码错误")
        return False
    print(chardet.detect(str.encode("打开灯"))["encoding"])
    text_=text_.encode("utf-8").decode("utf-8")
    if((text_.find("开")>=0)&(text_.find("灯")>=0)):
        if text_.find("红灯")>=0:
            GPIO.setup(18, GPIO.OUT)
            GPIO.output(18, False)
            print("操作 打开红灯")
            callback("打开红灯 操作成功")
        elif text_.find("绿灯")>=0:
            GPIO.setup(16, GPIO.OUT)
            GPIO.output(16, False)
            print("操作 打开绿灯")
            callback("打开绿灯 操作成功")
        elif text_.find("蓝灯")>=0:
            GPIO.setup(12, GPIO.OUT)
            GPIO.output(12, False)
            print("操作 打开蓝灯")
            callback("打开蓝灯 操作成功")
        elif text_.find("所有")>=0:
            GPIO.setup(12, GPIO.OUT)
            GPIO.setup(16, GPIO.OUT)
            GPIO.setup(18, GPIO.OUT)
            GPIO.output(12, False)
            GPIO.output(16, False)
            GPIO.output(18, False)
            print("操作 打开所有灯")
            callback("打开所有灯 操作成功")
        else:
            print("无操作")
            callback("没有这个灯")
        return False
    if((text_.find("开")>=0)&(text_.find("门")>=0)):
        if text_.find("厕所")>=0:
            if(get_identity().find("1")>=0):
                opendoor(35,True)
                callback("厕所门已 打开")
            else:
                callback("不认识你，不给你开门")
        else:
            callback("没有对应门")
        return False
    if((text_.find("关")>=0)&(text_.find("门")>=0)):
        if text_.find("厕所")>=0:
            opendoor(35,False)
            callback("厕所门已 关闭")
        else:
            callback("没有对应门")
        return False
    if((text_.find("关")>=0)&(text_.find("灯")>=0)):
        if text_.find("红灯")>=0:
            GPIO.setup(18, GPIO.OUT)
            GPIO.output(18, True)
            print("操作 关闭红灯")
            callback("关闭红灯 操作成功")
        elif text_.find("绿灯")>=0:
            GPIO.setup(16, GPIO.OUT)
            GPIO.output(16, True)
            print("操作 关闭绿灯")
            callback("关闭绿灯 操作成功")
        elif text_.find("蓝灯")>=0:
            GPIO.setup(12, GPIO.OUT)
            GPIO.output(12, True)
            print("操作 关闭蓝灯")
            callback("关闭蓝灯 操作成功")
        elif text_.find("所有")>=0:
            GPIO.setup(12, GPIO.OUT)
            GPIO.setup(16, GPIO.OUT)
            GPIO.setup(18, GPIO.OUT)
            GPIO.output(12, True)
            GPIO.output(16, True)
            GPIO.output(18, True)
            print("操作 所有灯")
            callback("关闭所有灯 操作成功")
        else:
            print("无操作")
            callback("没有这个灯")
        return False
    if((text_.find("关")>=0)&(text_.find("系统")>=0)):
            print("后会有期")
            callback("后会有期")
            sys.exit()
    if text_.find("退下吧")>=0:   
        os.system("python3 /home/pi/work/Linux_Cpp/xiaoluo/snowboy/demo.py /home/pi/work/Linux_Cpp/xiaoluo/snowboy/xiaotuxiaotu.pmdl /home/pi/work/Linux_Cpp/xiaoluo/snowboy/zmkm.pmdl &")
        sys.exit()  
    else:
        print("识别错误",text_)
        return True