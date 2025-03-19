import lvgl as lv
import time
from espidf import VSPI_HOST
from ili9XXX import ili9341
from xpt2046 import xpt2046
import fs_driver
from machine import I2C,Pin,ADC
import onewire, ds18x20
import DHT20
# ------------------------------ Initialize the screen --start------------------------


import lvgl as lv
import ui_images

pin25 = Pin(16, Pin.OUT)
i2c = I2C(scl=Pin(21), sda=Pin(22))
sensor = DHT20.DHT20(i2c)

WIDTH = 320
HEIGHT = 240

Tem = 0
Hum = 0


disp = ili9341(miso=12, mosi=13, clk=14, cs=15, dc=2, rst=-1, power=0, backlight=27, backlight_on=-1, power_on=1, rot=0x30,
        spihost=VSPI_HOST, mhz=60, factor=16, hybrid=True, width=WIDTH, height=HEIGHT,
        invert=False, double_buffer=True, half_duplex=False, initialize=True)


touch = xpt2046(cs=33, spihost=VSPI_HOST, mosi=-1, miso=-1, clk=-1, cal_x1 = 3656, cal_x0=219,cal_y0 = 423, cal_y1=3948,transpose = 0)


sensor.read_dht20()


def SetFlag( obj, flag, value):
    if (value):
        obj.add_flag(flag)
    else:
        obj.clear_flag(flag)
    return

_ui_comp_table = {}
_ui_comp_prev = None
_ui_name_prev = None
_ui_child_prev = None
_ui_comp_table.clear()

def _ui_comp_del_event(e):
    target = e.get_target()
    _ui_comp_table[id(target)].remove()

def ui_comp_get_child(comp, child_name):
    return _ui_comp_table[id(comp)][child_name]

def ui_comp_get_root_from_child(child, compname):
    for component in _ui_comp_table:
        if _ui_comp_table[component]["_CompName"]==compname:
            for part in _ui_comp_table[component]:
                if id(_ui_comp_table[component][part]) == id(child):
                    return _ui_comp_table[component]
    return None
def SetBarProperty(target, id, val):
   if id == 'Value_with_anim': target.set_value(val, lv.ANIM.ON)
   if id == 'Value': target.set_value(val, lv.ANIM.OFF)
   return

def SetPanelProperty(target, id, val):
   if id == 'Position_X': target.set_x(val)
   if id == 'Position_Y': target.set_y(val)
   if id == 'Width': target.set_width(val)
   if id == 'Height': target.set_height(val)
   return

def SetDropdownProperty(target, id, val):
   if id == 'Selected':
      target.set_selected(val)
   return

def SetImageProperty(target, id, val, val2):
   if id == 'Image': target.set_src(val)
   if id == 'Angle': target.set_angle(val2)
   if id == 'Zoom': target.set_zoom(val2)
   return

def SetLabelProperty(target, id, val):
   if id == 'Text': target.set_text(val)
   return

def SetRollerProperty(target, id, val):
   if id == 'Selected':
      target.set_selected(val, lv.ANIM.OFF)
   if id == 'Selected_with_anim':
      target.set_selected(val, lv.ANIM.ON)
   return

def SetSliderProperty(target, id, val):
   if id == 'Value_with_anim': target.set_value(val, lv.ANIM.ON)
   if id == 'Value': target.set_value(val, lv.ANIM.OFF)
   return

def ChangeScreen( src, fademode, speed, delay):
    lv.scr_load_anim(src, fademode, speed, delay, False)
    return

def DeleteScreen(src):
    return

def IncrementArc( trg, val):
    trg.set_value(trg.get_value()+val)
    lv.event_send(trg,lv.EVENT.VALUE_CHANGED, None)
    return

def IncrementBar( trg, val, anim):
    trg.set_value(trg.get_value()+val,anim)
    return

def IncrementSlider( trg, val, anim):
    trg.set_value(trg.get_value()+val,anim)
    lv.event_send(trg,lv.EVENT.VALUE_CHANGED, None)
    return

def KeyboardSetTarget( keyboard, textarea):
    keyboard.set_textarea(textarea)
    return

def ModifyFlag( obj, flag, value):
    if (value=="TOGGLE"):
        if ( obj.has_flag(flag) ):
            obj.clear_flag(flag)
        else:
            obj.add_flag(flag)
        return

    if (value=="ADD"):
        obj.add_flag(flag)
    else:
        obj.clear_flag(flag)
    return

def ModifyState( obj, state, value):
    if (value=="TOGGLE"):
        if ( obj.has_state(state) ):
            obj.clear_state(state)
        else:
            obj.add_state(state)
        return

    if (value=="ADD"):
        obj.add_state(state)
    else:
        obj.clear_state(state)
    return

def set_opacity(obj, v):
    obj.set_style_opa(v, lv.STATE.DEFAULT|lv.PART.MAIN)
    return

def SetTextValueArc( trg, src, prefix, postfix):
    trg.set_text(prefix+str(src.get_value())+postfix)
    return

def SetTextValueSlider( trg, src, prefix, postfix):
    trg.set_text(prefix+str(src.get_value())+postfix)
    return

def SetTextValueChecked( trg, src, txton, txtoff):
    if src.has_state(lv.STATE.CHECKED):
        trg.set_text(txton)
    else:
        trg.set_text(txtoff)
    return

def StepSpinbox( trg, val):
    if val==1 : trg.increment()
    if val==-1 : trg.decrement()
    lv.event_send(trg,lv.EVENT.VALUE_CHANGED, None)
    return

# COMPONENTS

 # COMPONENT Button2
def ui_Button2_create(comp_parent):
    cui_Button2 = lv.btn(comp_parent)
    cui_Button2.set_width(100)
    cui_Button2.set_height(50)
    cui_Button2.set_x(4)
    cui_Button2.set_y(32)
    cui_Button2.set_align( lv.ALIGN.CENTER)
    SetFlag(cui_Button2, lv.obj.FLAG.SCROLLABLE, False)
    SetFlag(cui_Button2, lv.obj.FLAG.SCROLL_ON_FOCUS, True)
    _ui_comp_table[id(cui_Button2)]= {"Button2" : cui_Button2, "_CompName" : "Button2"}
    return cui_Button2

# COMPONENTS

 # COMPONENT Button2
def ui_Button2_create(comp_parent):
    cui_Button2 = lv.btn(comp_parent)
    cui_Button2.set_width(100)
    cui_Button2.set_height(50)
    cui_Button2.set_x(4)
    cui_Button2.set_y(32)
    cui_Button2.set_align( lv.ALIGN.CENTER)
    SetFlag(cui_Button2, lv.obj.FLAG.SCROLLABLE, False)
    SetFlag(cui_Button2, lv.obj.FLAG.SCROLL_ON_FOCUS, True)
    _ui_comp_table[id(cui_Button2)]= {"Button2" : cui_Button2, "_CompName" : "Button2"}
    return cui_Button2

ui____initial_actions0 = lv.obj()

def Button1_eventhandler(event_struct):
   event = event_struct.code
   if event == lv.EVENT.CLICKED and True:
      pin25.value(1)
   return

def Button2_eventhandler(event_struct):
   event = event_struct.code
   if event == lv.EVENT.CLICKED and True:
      pin25.value(0)
   return

ui_Screen1 = lv.obj()
SetFlag(ui_Screen1, lv.obj.FLAG.SCROLLABLE, False)
ui_Screen1.set_style_bg_color(lv.color_hex(0xB6B3AA), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Screen1.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )

ui_Screen1.set_style_bg_img_src( ui_images.TemporaryImage, lv.PART.SCROLLBAR | lv.STATE.DEFAULT )




ui_Label3 = lv.label(ui_Screen1)
ui_Label3.set_text("TEMP:          °C")
ui_Label3.set_width(110)
ui_Label3.set_height(20)
ui_Label3.set_x(-49)
ui_Label3.set_y(-29)
ui_Label3.set_align( lv.ALIGN.CENTER)
ui_Label3.set_style_text_color(lv.color_hex(0x6713E3), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label3.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label3.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )



ui_Label4 = lv.label(ui_Screen1)
ui_Label4.set_text("HUM:           %")
ui_Label4.set_width(110)
ui_Label4.set_height(20)
ui_Label4.set_x(-48)
ui_Label4.set_y(33)
ui_Label4.set_align( lv.ALIGN.CENTER)
ui_Label4.set_style_text_color(lv.color_hex(0x6713E3), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label4.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label4.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )

ui_Label1 = lv.label(ui_Screen1)
ui_Label1.set_text(f"{Tem:.1f}")
ui_Label1.set_width(25)
ui_Label1.set_height(25)
ui_Label1.set_x(-28)
ui_Label1.set_y(-30)
ui_Label1.set_align( lv.ALIGN.CENTER)
ui_Label1.set_style_text_color(lv.color_hex(0xF9F2F2), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label1.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label1.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )

ui_Label2 = lv.label(ui_Screen1)
ui_Label2.set_text("50")
ui_Label2.set_width(25)
ui_Label2.set_height(25)
ui_Label2.set_x(-27)
ui_Label2.set_y(32)
ui_Label2.set_align( lv.ALIGN.CENTER)
ui_Label2.set_style_text_color(lv.color_hex(0xF9F2F2), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label2.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label2.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )

ui_Button1 = lv.btn(ui_Screen1)
ui_Button1.set_width(40)
ui_Button1.set_height(40)
ui_Button1.set_x(77)
ui_Button1.set_y(-34)
ui_Button1.set_align( lv.ALIGN.CENTER)
SetFlag(ui_Button1, lv.obj.FLAG.SCROLLABLE, False)
SetFlag(ui_Button1, lv.obj.FLAG.SCROLL_ON_FOCUS, True)
ui_Button1.set_style_bg_color(lv.color_hex(0xFFFFFF), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Button1.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Button1.set_style_bg_img_src( ui_images.ui_img_on_png, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Button1.set_style_bg_color(lv.color_hex(0xE71B1B), lv.PART.MAIN | lv.STATE.PRESSED )
ui_Button1.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.PRESSED )

ui_Button1.add_event_cb(Button1_eventhandler, lv.EVENT.ALL, None)
ui_Button2 = lv.btn(ui_Screen1)
ui_Button2.set_width(40)
ui_Button2.set_height(40)
ui_Button2.set_x(79)
ui_Button2.set_y(38)
ui_Button2.set_align( lv.ALIGN.CENTER)
SetFlag(ui_Button2, lv.obj.FLAG.SCROLLABLE, False)
SetFlag(ui_Button2, lv.obj.FLAG.SCROLL_ON_FOCUS, True)
ui_Button2.set_style_bg_color(lv.color_hex(0xFFFFFF), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Button2.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Button2.set_style_bg_img_src( ui_images.ui_img_off_png, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Button2.set_style_bg_color(lv.color_hex(0xBAB3B3), lv.PART.MAIN | lv.STATE.PRESSED )
ui_Button2.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.PRESSED )

ui_Button2.add_event_cb(Button2_eventhandler, lv.EVENT.ALL, None)
ui_Label5 = lv.label(ui_Screen1)
ui_Label5.set_text("Weather and Lamp")
ui_Label5.set_width(150)
ui_Label5.set_height(20)
ui_Label5.set_x(-2)
ui_Label5.set_y(-83)
ui_Label5.set_align( lv.ALIGN.CENTER)
ui_Label5.set_style_text_color(lv.color_hex(0xD72525), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label5.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )

class TEM_HUM():
    def __init__(self, ui_Screen1):
        # 读取DHT20传感器的温湿度值
        global Tem, Hum
        Tem = sensor.dht20_temperature()
        Hum = sensor.dht20_humidity()
        
        # 更新UI界面上的温湿度显示
        ui_Label1.set_text(f"{round(Tem)} °C")  # 更新温度显示
        ui_Label2.set_text(f"{round(Hum)} %")       # 更新湿度显示

# 确保TEM_HUM类在程序中被实例化，并且传入了ui_Screen1作为参数
def update_temperature_humidity():
    global Tem, Hum
    # 读取DHT20传感器的温湿度值
    sensor.read_dht20()  # 确保调用了read_dht20()来更新传感器数据
    Tem = sensor.dht20_temperature()
    Hum = sensor.dht20_humidity()
    
    # 更新UI界面上的温湿度显示
    ui_Label1.set_text(f"{round(Tem)}")
    ui_Label2.set_text(f"{round(Hum)}")
    
def main_loop():
    while True:
        update_temperature_humidity()
        time.sleep(1)  # 暂停10秒钟，然后再次更新

TEM_HUM(ui_Screen1)

# 其余代码保持不变


lv.scr_load(ui_Screen1)
main_loop()
