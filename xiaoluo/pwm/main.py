# Software PWM Servo.py

import RPi.GPIO as GPIO
import time

P_SERVO = 35 # GPIO端口号，根据实际修改
fPWM = 50  # Hz (软件PWM方式，频率不能设置过高)
a = 10
b = 2
pwm=None
def setup():
    global pwm
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(P_SERVO, GPIO.OUT)
    pwm = GPIO.PWM(P_SERVO, fPWM)
    pwm.start(0)

def setDirection(direction):
    duty = a / 180 * direction + b
    pwm.ChangeDutyCycle(duty)
    print("direction =", direction, "-> duty =", duty)
    time.sleep(1) 
   
print("starting")
setup()
setDirection(30)    
time.sleep(1) 
setDirection(90)    
time.sleep(1) 
GPIO.cleanup() 
print("done")