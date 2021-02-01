#sudo arecord -D "plughw:1,0" -d 5 temp.wav
#omxplayer -o local temp.wav
import iat
import time
import os
import tts
import socket
import Adafruit_PCA9685
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
def switch_page(msg):
    try:
        sk = socket.socket()
        sk.connect(('127.0.0.1',8080))
        sk.send(msg.encode("utf-8"))
        msg = sk.recv(1024).decode('utf-8')
        print("msg:",msg)
        sk.close()
        return msg
    except:
        print("人脸识别服务 发生异常")
        return "NULL"

def word_to_voice(text_):
    tts.tts(text_)
    switch_page(0)
    os.system("mplayer -af volume=10 demo.mp3")
    switch_page(1)


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
print('Moving servo on channel 0, press Ctrl-C to quit...')

def set_angle(channel,angle):
    i=round(angle*3.1+160)
    pwm.set_pwm(channel,0,i)



for i in range(0,70):
    set_angle(3,i)
    time.sleep(0.02)
if(get_identity().find("0")>=0):
    word_to_voice("不认识你")
    for i in range(70,0,-1):
        set_angle(3,i)
        time.sleep(0.02)
    switch_page("stack head:1")
else:
    switch_page("stack head:2")
    switch_page("sharkear")
    while True:
        word_to_voice("有话请讲")
        os.system("arecord -fS16_LE -r16000 -D plughw:1,0 -c1 -d 3 demo.pcm")
        res=iat.iat()






