#Make by Elecrow
#Web：www.elecrow.com
import lvgl as lv
import time
from espidf import VSPI_HOST
from ili9XXX import ili9341
from xpt2046 import xpt2046
import fs_driver
from machine import Pin,ADC
import onewire, ds18x20


WIDTH = 240
HEIGHT = 320



disp = ili9341(miso=12, mosi=13, clk=14, cs=15, dc=2, rst=-1, power=0, backlight=27, backlight_on=-1, power_on=1, rot=0x80,
        spihost=VSPI_HOST, mhz=60, factor=16, hybrid=True, width=WIDTH, height=HEIGHT,
        invert=False, double_buffer=True, half_duplex=False, initialize=True)


touch = xpt2046(cs=25, spihost=VSPI_HOST, mosi=-1, miso=-1, clk=-1, cal_y0 = 423, cal_y1=3948)

scr = lv.obj()  # scr====> screen 
fs_drv = lv.fs_drv_t()
fs_driver.fs_register(fs_drv, 'S')
scr = lv.scr_act()
scr.clean()
ps2_y = ADC(Pin(25))
ps2_y.atten(ADC.ATTN_11DB) 
s = 0
pin16 = Pin(16, Pin.OUT)

class MyWidget():
    def __init__(self, scr):
       
        self.meter = lv.meter(scr)
        self.meter.set_pos(20, 50) 
        
        self.meter.set_size(200, 200)  
        
        label = lv.label(self.meter)
        label.set_text("Light intensity")
        label.align( lv.ALIGN.CENTER, 0, 75)
        
       
        scale = self.meter.add_scale()
        
        self.meter.set_scale_ticks(scale, 51, 2, 10, lv.palette_main(lv.PALETTE.GREY))
        
        self.meter.set_scale_major_ticks(scale, 10, 4, 15, lv.color_black(), 20)

        
        blue_arc = self.meter.add_arc(scale, 2, lv.palette_main(lv.PALETTE.BLUE), 0)
        self.meter.set_indicator_start_value(blue_arc, 0)
        self.meter.set_indicator_end_value(blue_arc, 100)

       
        blue_arc_scale = self.meter.add_scale_lines(scale, lv.palette_main(lv.PALETTE.BLUE), lv.palette_main(lv.PALETTE.BLUE), False, 0)
        self.meter.set_indicator_start_value(blue_arc_scale, 0)
        self.meter.set_indicator_end_value(blue_arc_scale, 20)

        
        red_arc = self.meter.add_arc(scale, 2, lv.palette_main(lv.PALETTE.RED), 0)
        self.meter.set_indicator_start_value(red_arc, 80)
        self.meter.set_indicator_end_value(red_arc, 100)

        
        red_arc_scale = self.meter.add_scale_lines(scale, lv.palette_main(lv.PALETTE.RED), lv.palette_main(lv.PALETTE.RED), False, 0)
        self.meter.set_indicator_start_value(red_arc_scale, 80)
        self.meter.set_indicator_end_value(red_arc_scale, 100)

        
        self.indic = self.meter.add_needle_line(scale, 4, lv.palette_main(lv.PALETTE.GREY), -10)
        
       
        a = lv.anim_t()
        a.init()
        a.set_var(self.indic)
        val_y = ps2_y.read()
        a.set_values(0, 100)
        a.set_time(2000)
        a.set_repeat_delay(100)
        a.set_playback_time(500)
        a.set_playback_delay(100)
        a.set_repeat_count(lv.ANIM_REPEAT_INFINITE)
        a.set_custom_exec_cb(self.set_value)
        lv.anim_t.start(a)
        
    def set_value(self, anmi_obj, value):
       
       
        ps2_y = ADC(Pin(25))
        ps2_y_value = ps2_y.read()
        ps2_y = int(ps2_y_value)
        s = int((((ps2_y - 0) * 100) / 4095) + 0)
        if s > 60:
            pin16.value(0)
            label2.set_text(" ")
            label3.set_text("OFF")  
        else:
            pin16.value(1)
            label2.set_text("ON")
            label3.set_text(" ")  
        self.meter.set_indicator_value(self.indic, s)
       
btn2 = lv.btn(scr)  
btn2.set_size(40, 30) 
btn2.align(lv.ALIGN.CENTER,-50,125)  
style_btn2 = lv.style_t()
style_btn2.init()
style_btn2.set_bg_color(lv.color_hex(0x00FF00))

label2 = lv.label(btn2)  
label2.set_text("ON")  
label2.center() 

btn3 = lv.btn(scr)  
btn3.set_size(40, 30)  
btn3.align(lv.ALIGN.CENTER,50,125)  
style_btn3 = lv.style_t()
style_btn3.init()

style_btn3.set_bg_color(lv.color_hex(0xFFFF00))

label3 = lv.label(btn3)  
label3.set_text("OFF") 
label3.center()  


MyWidget(scr)


lv.scr_load(scr)



try:
    from machine import WDT
    wdt = WDT(timeout=1000)  # enable it with a timeout of 2s
    print("Hint: Press Ctrl+C to end the program")
    while True:
        wdt.feed()
        time.sleep(0.9)
except KeyboardInterrupt as ret:
    print("The program stopped running, ESP32 has restarted...")
    time.sleep(10)


