from __future__ import division
import time
import Adafruit_PCA9685
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
while True:
    for i in range(0,70):
        set_angle(3,i)
        time.sleep(0.02)
    for i in range(70,0,-1):
        set_angle(3,i)
        time.sleep(0.02)
#小兔子1
while True:
    for i in range(30,100):
        set_angle(1,i)
        time.sleep(0.01)
    for i in range(100,30,-1):
        set_angle(1,i)
        time.sleep(0.01)
#小兔子2
while True:
    for i in range(40,150):
        set_angle(0,i)
        time.sleep(0.01)
    for i in range(150,40,-1):
        set_angle(0,i)
        time.sleep(0.01)
#脑袋
while True:
    for i in range(20,80):
        set_angle(13,i)
        time.sleep(0.01)
    for i in range(80,20,-1):
        set_angle(13,i)
        time.sleep(0.01)
#耳朵
while True:
    for i in range(0,20):
        set_angle(14,i)
        set_angle(15,20-i)
    for i in range(20,0,-1):
        set_angle(14,i)
        set_angle(15,20-i)
#伸出脑袋
while True:
    for i in range(0,90):
        set_angle(0,i)
        set_angle(2,i)
        time.sleep(0.01)
    for i in range(90,0,-1):
        set_angle(0,i)
        set_angle(2,i)
        time.sleep(0.01)
while True:
    for i in range(150,600):
        pwm.set_pwm(0,0,i)
        pwm.set_pwm(1,0,i)
        pwm.set_pwm(2,0,i)
        time.sleep(0.01)
    for i in range(600,150,-1):
        pwm.set_pwm(0,0,i)
        pwm.set_pwm(1,0,i)
        pwm.set_pwm(2,0,i)
        time.sleep(0.01)
