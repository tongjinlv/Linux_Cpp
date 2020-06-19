
import tts




def app(text_):
    text_=text_.encode("utf-8").decode("utf-8")
    if(text_=="打开灯"):
        tts.tts("好的")
        return False
    else:
        print("识别错误",text_)
        return True