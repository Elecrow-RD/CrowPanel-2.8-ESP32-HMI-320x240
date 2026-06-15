#Make by Elecrow
#Web：www.elecrow.com
import lvgl as lv
import time
from espidf import VSPI_HOST
from ili9XXX import ili9341
from xpt2046 import xpt2046
import fs_driver
from machine import Pin
import onewire, ds18x20

# ------------------------------ Initialize the screen --start------------------------

WIDTH = 240
HEIGHT = 320


# Creating the display object
disp = ili9341(miso=12, mosi=13, clk=14, cs=15, dc=2, rst=-1, power=0, backlight=27, backlight_on=-1, power_on=1, rot=0x80,
        spihost=VSPI_HOST, mhz=60, factor=16, hybrid=True, width=WIDTH, height=HEIGHT,
        invert=False, double_buffer=True, half_duplex=False, initialize=True)

# Create a touch screen object
touch = xpt2046(cs=25, spihost=VSPI_HOST, mosi=-1, miso=-1, clk=-1, cal_y0 = 423, cal_y1=3948)
# --------------------------------stop------------------------


# 1. Create a display screen. Will need to display the component added to the screen to display
scr = lv.obj()  # scr====> screen
fs_drv = lv.fs_drv_t()
fs_driver.fs_register(fs_drv, 'S')
scr = lv.scr_act()
scr.clean()


# 2. Encapsulate the component to display
class MyWidget():
    def __init__(self, scr):
        # 1. Create the dashboard object
        self.meter = lv.meter(scr)
        self.meter.center()  
        self.meter.set_size(200, 200)  # width: 200 height: 200

        # 2. To create calibration object
        scale = self.meter.add_scale()
        
        self.meter.set_scale_ticks(scale, 51, 2, 10, lv.palette_main(lv.PALETTE.GREY))
      
        self.meter.set_scale_major_ticks(scale, 10, 4, 15, lv.color_black(), 20)

        # 3. Add warning scale line
      
        blue_arc = self.meter.add_arc(scale, 2, lv.palette_main(lv.PALETTE.BLUE), 0)
        self.meter.set_indicator_start_value(blue_arc, 0)
        self.meter.set_indicator_end_value(blue_arc, 20)

     
        blue_arc_scale = self.meter.add_scale_lines(scale, lv.palette_main(lv.PALETTE.BLUE), lv.palette_main(lv.PALETTE.BLUE), False, 0)
        self.meter.set_indicator_start_value(blue_arc_scale, 0)
        self.meter.set_indicator_end_value(blue_arc_scale, 20)

     
        red_arc = self.meter.add_arc(scale, 2, lv.palette_main(lv.PALETTE.RED), 0)
        self.meter.set_indicator_start_value(red_arc, 80)
        self.meter.set_indicator_end_value(red_arc, 100)

       
        red_arc_scale = self.meter.add_scale_lines(scale, lv.palette_main(lv.PALETTE.RED), lv.palette_main(lv.PALETTE.RED), False, 0)
        self.meter.set_indicator_start_value(red_arc_scale, 80)
        self.meter.set_indicator_end_value(red_arc_scale, 100)

        # 4. meter needle
        self.indic = self.meter.add_needle_line(scale, 4, lv.palette_main(lv.PALETTE.GREY), -10)
        
        # 5. Creating animated objects
        a = lv.anim_t()
        a.init()
        a.set_var(self.indic)
        a.set_values(0, 100)
        a.set_time(2000)
        a.set_repeat_delay(100)
        a.set_playback_time(500)
        a.set_playback_delay(100)
        a.set_repeat_count(lv.ANIM_REPEAT_INFINITE)
        a.set_custom_exec_cb(self.set_value)
        lv.anim_t.start(a)

    def set_value(self, anmi_obj, value):
        """Animation callbacks"""
        self.meter.set_indicator_value(self.indic, value)
    

# 3. Create the component to display
MyWidget(scr)

# 4. Displays the contents of the screen object
lv.scr_load(scr)


# ------------------------------ Guard dog to restart ESP32 equipment --start------------------------
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
# ------------------------------ Guard dog to restart ESP32 equipment --stop-------------------------

