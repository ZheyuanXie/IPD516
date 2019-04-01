import threading
import time

def t1():
    print("i'm thread 1")
    return

def t2():
    print("i'm thread 2")
    time.sleep(3)
    return 

thread1 = threading.Thread(None, t1, None)
thread2 = threading.Thread(None, t2, None)
thread1.start()
thread2.start()
thread1.join()
thread2.join()
print("terminated")
