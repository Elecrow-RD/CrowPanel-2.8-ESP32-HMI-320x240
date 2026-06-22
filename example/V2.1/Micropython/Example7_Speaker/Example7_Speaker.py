import math
import time
from machine import Pin, DAC


NOTE_C5 = 523
NOTE_D5 = 587
NOTE_E5 = 659
NOTE_F5 = 698
NOTE_G5 = 784
NOTE_A5 = 880


TwinkleTwinkle = [
    NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_A5, NOTE_G5,
    NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, NOTE_D5, NOTE_C5,
    NOTE_G5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5,
    NOTE_G5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5,
    NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_A5, NOTE_G5,
    NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, NOTE_D5, NOTE_C5
]

DURATION = 500  

dac = DAC(Pin(26))

def play_tone(frequency, duration_ms):

    if frequency == 0:
        time.sleep_ms(duration_ms)
        return
    
    SAMPLE_RATE = 10000  
    samples_per_cycle = int(SAMPLE_RATE / frequency)  

    wave_table = []
    for i in range(samples_per_cycle):
        angle = 2 * math.pi * i / samples_per_cycle
        sample = int(128 + 127 * math.sin(angle))

        sample = max(0, min(255, sample))
        wave_table.append(sample)
    

    total_samples = int(SAMPLE_RATE * duration_ms / 1000)
    

    for i in range(total_samples):
        dac.write(wave_table[i % samples_per_cycle])
        time.sleep_us(100)  

def play_melody(melody, duration_ms):
    for note in melody:
        play_tone(note, duration_ms)
        time.sleep_ms(50)  

def setup():
    print("play music...")
    play_melody(TwinkleTwinkle, DURATION)

def loop():
    while True:
        pass

setup()
loop()