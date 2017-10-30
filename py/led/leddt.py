import RPi.GPIO as GPIO
import time

#GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(13, GPIO.OUT)
GPIO.setup(15,GPIO.OUT)
while True:
    GPIO.output(13, GPIO.HIGH)
    GPIO.output(15,GPIO.LOW)
    time.sleep(1)
    GPIO.output(13, GPIO.HIGH)
    GPIO.output(15,GPIO.HIGH)
    time.sleep(1)
