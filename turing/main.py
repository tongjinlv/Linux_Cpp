#sudo arecord -D "plughw:1,0" -d 5 temp.wav
#omxplayer -o local temp.wav
import iat

import os

while True:
    print("请说话")
    os.system("arecord -fS16_LE -r16000 -D plughw:1,0 -c1 -d 5 demo.pcm")
    res=iat.iat()


