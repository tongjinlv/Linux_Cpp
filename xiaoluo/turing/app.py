
import tts
import chardet
import RPi.GPIO as GPIO
import os,sys
GPIO.setmode(GPIO.BOARD)

def callback(text_):
    tts.tts(text_)
    os.system("mplayer -af volume=20 demo.mp3")
def app(text_):
    if(chardet.detect(str.encode(text_))["encoding"]!="utf-8"):
        print("编码错误")
        return False
    print(chardet.detect(str.encode("打开灯"))["encoding"])
    text_=text_.encode("utf-8").decode("utf-8")
    if text_.find("开")>=0:
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
        else:
            print("无操作")
            callback("没有这个灯")
        return False
    if text_.find("关")>=0:
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
        elif text_.find("系统")>=0:
            GPIO.setup(12, GPIO.OUT)
            GPIO.output(12, True)
            print("后会有期")
            callback("后会有期")
            sys.exit()
    else:
        print("识别错误",text_)
        return True