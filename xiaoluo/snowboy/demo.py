import snowboydecoder
import sys
import signal

interrupted = False


def signal_handler(signal, frame):
    global interrupted
    interrupted = True


def interrupt_callback():
    global interrupted
    return interrupted

if len(sys.argv) != 3:
    print("Error: need to specify 2 model names")
    print("Usage: python demo.py 1st.model 2nd.model")
    sys.exit(-1)

def detected():
        print("Great! I have recognized your words.\n")

callbacks = [lambda: detected(),
             lambda: snowboydecoder.play_audio_file(snowboydecoder.DETECT_DONG)]
models = sys.argv[1:]
sensitivity = [0.5]*len(models)
# capture SIGINT signal, e.g., Ctrl+C
signal.signal(signal.SIGINT, signal_handler)

detector = snowboydecoder.HotwordDetector(models, sensitivity=sensitivity)
print('Listening... Press Ctrl+C to exit')

# main loop
#detector.start(detected_callback=snowboydecoder.play_audio_file,
detector.start(detected_callback=callbacks,
               interrupt_check=interrupt_callback,
               sleep_time=0.03)

detector.terminate()
print("run file over")
