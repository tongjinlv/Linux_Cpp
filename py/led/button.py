import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BOARD)
GPIO.setup(11,GPIO.IN,GPIO.PUD_UP)
print(GPIO.input(11))
