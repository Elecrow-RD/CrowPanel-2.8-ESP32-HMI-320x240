#Make by Elecrow
#Web：www.elecrow.com
import lvgl as lv
import time
from espidf import VSPI_HOST
from ili9XXX import ili9341
from xpt2046 import xpt2046



WIDTH = 240
HEIGHT = 320



disp = ili9341(miso=12, mosi=13, clk=14, cs=15, dc=2, rst=-1, power=0, backlight=27, backlight_on=-1, power_on=1, rot=0x80,
        spihost=VSPI_HOST, mhz=60, factor=16, hybrid=True, width=WIDTH, height=HEIGHT,
        invert=False, double_buffer=True, half_duplex=False, initialize=True)


touch = xpt2046(cs=33, spihost=VSPI_HOST, mosi=-1, miso=-1, clk=-1, cal_y0 = 423, cal_y1=3948)




scr = lv.obj()



class CounterBtn():
    def __init__(self, scr):
        self.cnt = 0
        btn = lv.btn(scr)  
        btn.set_size(120, 50) 
        btn.align(lv.ALIGN.CENTER,0,0)  
        btn.add_event_cb(self.btn_event_cb, lv.EVENT.ALL, None)  
        label = lv.label(btn) 
        label.set_text("Button")
        label.set_style_text_font(lv.font_montserrat_12, 0)
        label.center() 

    def btn_event_cb(self, evt):
        code = evt.get_code()  
        btn = evt.get_target()  
        if code == lv.EVENT.CLICKED:
            self.cnt += 1

        
        label = btn.get_child(0)
        label.set_style_text_font(lv.font_montserrat_12, 0)
        label.set_text("Button: " + str(self.cnt))  



counterBtn = CounterBtn(scr)


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


