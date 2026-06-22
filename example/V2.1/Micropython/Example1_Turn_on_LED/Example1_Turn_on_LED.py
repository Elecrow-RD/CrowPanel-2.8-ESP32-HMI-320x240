#Make by Elecrow
#Web：www.elecrow.com

import time
from machine import Pin
pin25 = Pin(25, Pin.OUT)
while True:

    pin25.value(1)
    time.sleep(0.5)
    pin25.value(0)
    time.sleep(0.5)