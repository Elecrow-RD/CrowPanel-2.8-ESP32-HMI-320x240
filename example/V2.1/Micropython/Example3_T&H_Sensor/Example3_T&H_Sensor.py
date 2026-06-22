import time
import DHT20
from machine import I2C, Pin
i2c = I2C(scl=Pin(21), sda=Pin(22))
sensor = DHT20.DHT20(i2c)

while True:
    sensor.read_dht20()
    print(sensor.dht20_temperature())
    print(sensor.dht20_humidity())
    time.sleep_ms(4000) 