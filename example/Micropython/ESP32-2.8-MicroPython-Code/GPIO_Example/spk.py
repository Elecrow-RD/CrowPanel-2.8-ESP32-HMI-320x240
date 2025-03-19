import math
import time
from machine import Pin, DAC

# 定义音符频率 (Hz)
NOTE_C5 = 523
NOTE_D5 = 587
NOTE_E5 = 659
NOTE_F5 = 698
NOTE_G5 = 784
NOTE_A5 = 880

# 定义旋律 (小星星)
TwinkleTwinkle = [
    NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_A5, NOTE_G5,
    NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, NOTE_D5, NOTE_C5,
    NOTE_G5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5,
    NOTE_G5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5,
    NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_A5, NOTE_G5,
    NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, NOTE_D5, NOTE_C5
]

# 每个音符的持续时间 (毫秒)
DURATION = 500

# 初始化DAC，使用GPIO 26
dac = DAC(Pin(26))

def play_tone(frequency, duration):
    if frequency == 0:
        time.sleep_ms(duration)
        return
    samples_per_cycle = 100
    for i in range(duration):
        for j in range(samples_per_cycle):
            dac.write(int(128 + 127 * math.sin(2 * math.pi * j / samples_per_cycle * frequency / 1000)))
            time.sleep_us(10)

def play_melody(melody, duration):
    for note in melody:
        play_tone(note, duration)
        time.sleep_ms(50)

def setup():
    print("开始播放音乐...")
    play_melody(TwinkleTwinkle, DURATION)

def loop():
    while True:
        pass

setup()
loop()
