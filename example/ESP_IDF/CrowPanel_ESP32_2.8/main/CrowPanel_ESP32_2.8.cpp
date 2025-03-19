#include "Arduino.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <examples/lv_examples.h>
#include <demos/lv_demos.h>
#include <DHT20.h>
//#include <Wire.h>
/*更改屏幕分辨率*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 8 ];

TFT_eSPI lcd = TFT_eSPI(); /* TFT实例 */
DHT20 dht20;
//UI
#include "ui.h"
uint16_t calData[5] = {189, 3416, 359, 3439, 1};
int led;
/* 显示器刷新 */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  lcd.startWrite();
  lcd.setAddrWindow( area->x1, area->y1, w, h );
  lcd.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  lcd.endWrite();

  lv_disp_flush_ready( disp );
}

uint16_t touchX, touchY;
/*读取触摸板*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
  bool touched = lcd.getTouch( &touchX, &touchY, 600);
  if ( !touched )
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*设置坐标*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print( "Data x " );
    Serial.println( touchX );

    Serial.print( "Data y " );
    Serial.println( touchY );
  }
}


extern "C" void app_main()
{
  Serial.begin(115200);
  //IO口引脚
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  
  Wire.begin(22, 21);
  dht20.begin();
  //lvgl初始化
  lv_init(); 

  //LCD初始化
  lcd.begin();          /*初始化*/
  lcd.fillScreen(TFT_BLACK);
  delay(300);
  //背光引脚
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  lcd.setRotation(1); /* 旋转 */

  //校准模式。一是四角定位、二是直接输入模拟数值直接定位
  //屏幕校准
  //  touch_calibrate();
  //lcd.setTouch( calData );

  lcd.setTouch(calData);
  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 8 );

  /*初始化显示*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*将以下行更改为显示分辨率*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*初始化（虚拟）输入设备驱动程序*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );
  ui_init();//开机UI界面
  while (1)
  {
    Serial.print(led);
    char DHT_buffer[6];
    int a = (int)dht20.getTemperature();
    int b = (int)dht20.getHumidity();
    snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", a);
    lv_label_set_text(ui_Label1, DHT_buffer);
    snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", b);
    lv_label_set_text(ui_Label2, DHT_buffer);
    if(led == 1)
      digitalWrite(25, HIGH);
    if(led == 0)
      digitalWrite(25, LOW);
    lv_timer_handler();
    Serial.println("Setup done");
  }
}
