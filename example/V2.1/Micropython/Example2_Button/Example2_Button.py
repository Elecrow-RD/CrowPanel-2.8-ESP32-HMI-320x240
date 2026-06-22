from machine import Pin
import time

led = Pin(25, Pin.OUT)


button = Pin(16, Pin.IN, Pin.PULL_UP)


led.value(0)

while True:
    
    if not button.value():
        
        led.value(1)
    else:
        
        led.value(0)
    
    time.sleep(0.1)