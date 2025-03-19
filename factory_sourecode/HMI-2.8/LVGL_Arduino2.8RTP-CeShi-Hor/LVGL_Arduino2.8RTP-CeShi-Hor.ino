#include <lvgl.h>
#include <TFT_eSPI.h>
#include <demos/lv_demos.h>
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#endif
#include <WiFi.h>
#include "MusicDefinitions.h"
#include "SoundData.h"
#include "XT_DAC_Audio.h"

#include "BLEDevice.h"              //BLE驱动库
#include "BLEServer.h"              //BLE蓝牙服务器库
#include "BLEUtils.h"               //BLE实用程序库
#include "BLE2902.h"                //特征添加描述符库
#include <BLECharacteristic.h>      //BLE特征函数库
BLEAdvertising* pAdvertising = NULL;
BLEServer* pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic* pCharacteristic = NULL;
#define bleServerName "Wizee-Series-2.8"  //BLE服务器的名称
#define SERVICE_UUID "6479571c-2e6d-4b34-abe9-c35116712345"  //服务的UUID
#define CHARACTERISTIC_UUID "826f072d-f87c-4ae6-a416-6ffdcaa02d73"

//EEPROM
#include <EEPROM.h>
#include <WiFi.h>
String ssid_s = "MERCURY_D268G"; //  输入你的WIFI名称
String password_s = "pba5ayzk"; //  输入你的WIFI密码
String ssid;
String password;
String e_flag = "U"; //如果想要修改已保存的WIFI信息，改下这个字母就可以
char sd[30];
char pd[30];

//UI
#include "ui.h"
#include <Ticker.h>          //Call the ticker. H Library
Ticker ticker1;
static int first_flag = 0;
extern int zero_clean;
extern int goto_widget_flag;
extern int bar_flag;
extern lv_obj_t * ui_MENU;
extern lv_obj_t * ui_TOUCH;
extern lv_obj_t * ui_JIAOZHUN;
extern lv_obj_t * ui_Label2;
static lv_obj_t * ui_Label;//TOUCH界面label
static lv_obj_t * ui_Label3;//TOUCH界面label3
static lv_obj_t * ui_Labe2;//Menu界面进度条label
static lv_obj_t * bar;//Menu界面进度条
static int val = 100;

char buf[128] = {};
int bufindex = 0;
int wifi_close_flag = 0;
char *info[128] = {};
int wifi_flag = 0;
int i = 0;
int touch_flag = 0;
//IIC
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C  u8g2(U8G2_R0, /* clock=*/ 21, /* data=*/ 22, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED

//2.8
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18
#define SD_CS 5


//SPEAK
// Data for the melody. Note followed by optional change in playing length in 1/4 beats. See documentation for more details
int8_t PROGMEM TwinkleTwinkle[] = {
  NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_A5, NOTE_G5, BEAT_2,
  NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, NOTE_D5, NOTE_C5, BEAT_2,
  NOTE_G5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, BEAT_2,
  NOTE_G5, NOTE_G5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, BEAT_2,
  NOTE_C5, NOTE_C5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_A5, NOTE_G5, BEAT_2,
  NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_D5, NOTE_D5, NOTE_C5, BEAT_4,
  NOTE_SILENCE, BEAT_5, SCORE_END
};
XT_DAC_Audio_Class DacAudio(26, 0);         // Create the main player class object. Use GPIO 25, one of the 2 DAC pins and timer 0
XT_Wav_Class ForceWithYou(Force);           // create WAV object and pass in the WAV data
XT_MusicScore_Class Music(TwinkleTwinkle, TEMPO_ALLEGRO, INSTRUMENT_PIANO); // The music score object, pass in the Music data
XT_Sequence_Class Sequence;                // The sequence object, you add your sounds above to this object (see setup below)

/*更改屏幕分辨率*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 10 ];

TFT_eSPI lcd = TFT_eSPI(); /* TFT实例 */

bool connected_state = false;   //创建设备连接标识符

class MyServerCallbacks: public BLEServerCallbacks  //创建连接和断开调用类
{
    void onConnect(BLEServer *pServer)//开始连接函数
    {
      connected_state = true;   //设备正确连接
    }
    void onDisconnect(BLEServer *pServer)//断开连接函数
    {
      connected_state = false;  //设备连接错误
    }

};

//_______________________
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


unsigned char buffer[256]; // buffer array for data recieve over serial port
int count = 0;   // counter for buffer array
void clearBufferArray()              // function to clear buffer array
{
  for (int i = 0; i < count; i++)
  {
    buffer[i] = NULL; // clear all index of array with command NULL
  }
}

char CloseData;
int NO_Test_Flag = 0;
int Test_Flag = 0;
int Close_Flag = 0;
uint16_t calData[5] = { 189, 3416, 359, 3439, 1 };
void callback1()  //Callback function
{
  if (bar_flag == 6)
  {
    if (val > 1)
    {
      val--;
      lv_bar_set_value(bar, val, LV_ANIM_OFF);
      lv_label_set_text_fmt(ui_Labe2, "%d %%", val);
    }
    else
    {
      lv_obj_clear_flag(ui_touch, LV_OBJ_FLAG_CLICKABLE);
      lv_label_set_text(ui_Labe2, "Loading");
      delay(150);
      val = 100;
      bar_flag = 0; //停止进度条标志
      goto_widget_flag = 1; //进入widget标志

    }
  }
}


//***********************************EEPROM*************************//
String calDataone;
String calDatatwo;
String calDatathree;
String calDatafour;
String calDatafive;

//读取或保存wifi等信息；
void save_or_read_wifi(void) {
  EEPROM.begin(1024);
  if (get_string(1, 0) == e_flag) { //如果第0位字符为“M”，则表示已储存WIFI信息
    Serial.println("已储存WIFI信息");
    //提取信息
    ssid = get_string(EEPROM.read(10), 15);
    password = get_string(EEPROM.read(50), 55);
    //      calDataone=get_string(EEPROM.read(20),10);
    //      calDatatwo=get_string(EEPROM.read(20),30);
    //      calDatathree=get_string(EEPROM.read(20),50);
    //      calDatafour=get_string(EEPROM.read(20),70);
    //      calDatafive=get_string(EEPROM.read(20),90);
  }
  else {
    Serial.println("未储存有WIFI信息,将储存wifi信息");
    //保存信息
    set_string(10, 15, ssid_s, 1); //保存wifi名称
    set_string(50, 55, password_s, 1); //保存wifi名称
    //    set_string(10,10,(char*)calData[0],1); //保存wifi名称
    //    set_string(10,30,(char*)calData[1],1);
    //    set_string(10,50,(char*)calData[2],1);
    //    set_string(10,70,(char*)calData[3],1);
    //    set_string(10,90,(char*)calData[4],1);

    set_string(1, 0, e_flag, 0); //录入"N"字符
    Serial.println("完成储存WIFI信息");
    //    Serial.println("即将重启");
    //    ESP.restart();
  }

  EEPROM.end();
}

//用EEPROM的a位保存字符串的长度，字符串的从EEPROM的b位开始保存，str为要保存的字符串，s为是否保存字符串长度
void set_string(int a, int b, String str, int s)
{
  if (s)EEPROM.write(a, str.length()); //EEPROM第a位，写入str字符串的长度
  //通过一个for循环，把str所有数据，逐个保存在EEPROM
  for (int i = 0; i < str.length(); i++) {
    EEPROM.write(b + i, str[i]);
  }
  EEPROM.commit();  //执行保存EEPROM

}

//获取指定EEPROM位置的字符串，a是字符串长度，b是起始位，从EEPROM的b位开始读取
String get_string(int a, int b) {
  String data = "";
  //通过一个for循环，从EEPROM中逐个取出每一位的值，并连接起来
  for (int i = 0; i < a; i++) {
    data += char(EEPROM.read(b + i));
  }
  return data;
}

void setup()
{
  Serial.begin( 115200 ); /*初始化串口*/
  Serial2.begin( 9600 ); /*初始化串口2*/

  //EEPROM
  //  Serial.println("");
  //  save_or_read_wifi();//读取或者保存wifi信息
  //  Serial.println("wifi信息");
  //  Serial.print("ssid:");
  //  Serial.println(ssid);
  //  Serial.print("password:");
  //  Serial.println(password);

  //IO口引脚
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);



  //OLED
  //  u8g2.begin();
  //  u8g2.enableUTF8Print();        // enable UTF8 support for the Arduino print() function
  //  u8g2.setFont(u8g2_font_ncenB14_tr);
  //  u8g2.setFontDirection(0);
  //  for (int i = 128; i > -78; i -= 20)
  //  {
  //    u8g2.firstPage();
  //    do {
  //      u8g2.drawStr(i, 25, "ELECROW");
  //      delay(2);
  //    } while ( u8g2.nextPage() );
  //  }

  //lvgl初始化
  lv_init();

  //LCD初始化
  lcd.begin();          /*初始化*/
  lcd.setRotation(1); /* 旋转 */
  lcd.fillScreen(TFT_BLACK);
  delay(200);
  //背光引脚
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  delay(100);

  //SD卡
  //  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  //  delay(100);
  //  if (SD_init() == 1)
  //  {
  //    Serial.println("SD卡初始化失败！");
  //  }
  //  else
  //    Serial.println("SD卡初始化成功");
  //  delay(2000);

  //校准模式。一是四角定位、二是直接输入模拟数值直接定位
  //屏幕校准
  //  EEPROM.begin(1024);
  //  Serial.print("666");
  //  if(get_string(1,0) == e_flag)
  lcd.setTouch( calData );
  //    else
  //    touch_calibrate();


  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 10 );

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
    if (goto_widget_flag == 1)//进入widget
    {
      if (ticker1.active() == true)
      {
        ticker1.detach();
      }
      goto_widget_flag = 0;
      delay(300);
      break;
    }

    if (goto_widget_flag == 3)//进入触摸界面，先把进度条线程关闭
    {
      bar_flag = 0; //停止进度条标志
      if (ticker1.active() == true)
      {
        ticker1.detach();
      }
      if (first_flag == 0 || first_flag == 1)
      {
        label_xy();
        first_flag = 2;
      }
      if (zero_clean == 1)
      {
        touchX = 0;
        touchY = 0;
        zero_clean = 0;
      }
      lv_label_set_text(ui_Label, "Touch Adjust:");
      lv_label_set_text_fmt(ui_Label3, "%d  %d", touchX, touchY); //显示触摸信息
    }

    if (goto_widget_flag == 4)//触摸界面返回到Menu界面,使进度条加满
    {
      val = 100;
      delay(100);
      ticker1.attach_ms(30, callback1);//每35ms调用callback1
      goto_widget_flag = 0;
    }

    if (goto_widget_flag == 5) //触发校准信号
    {
      lv_scr_load_anim(ui_touch_calibrate, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      lv_timer_handler();
      lv_timer_handler();
      delay(100);
      touch_calibrate();//触摸校准
      lcd.setTouch( calData );
      lv_scr_load_anim(ui_TOUCH, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      lv_timer_handler();
      delay(100);
      goto_widget_flag = 3; //进入触摸界面标志
      touchX = 0;
      touchY = 0;
    }

    if (bar_flag == 6)//刚开机进入Menu界面时运行进度条一次，之后就不再运行
    {
      if (first_flag == 0)
      {
        lv_example_bar();
        ticker1.attach_ms(30, callback1);//每35ms调用callback1
        first_flag = 1;
      }
    }

    lv_timer_handler();
  }


  lcd.fillScreen(TFT_BLACK);
  lv_demo_widgets();//主UI界面
  Serial.println( "Setup done" );


}

void loop()
{

  while (NO_Test_Flag == 0)
  {
    lv_timer_handler();
    //    uint16_t x, y;
    //    if (lcd.getTouch(&x, &y))
    //    {
    //      Serial.print( "Data x :" );
    //      Serial.println( x );
    //
    //      Serial.print( "Data y :" );
    //      Serial.println( y );
    //      lcd.setCursor(0, 100, 4);
    //      lcd.printf("Please Touch");
    //      lcd.setCursor(120, 200, 4);
    //      lcd.printf("               ");
    //      lcd.setCursor(120, 230, 4);
    //      lcd.printf("               ");
    //
    //      lcd.setCursor(100, 200, 4);
    //      lcd.printf("x: %i     ", x);
    //      lcd.setCursor(100, 230, 4);
    //      lcd.printf("y: %i    ", y);
    //    }
    CloseData = Serial.read();
    if (CloseData == 'b')  //Exit
    {
      NO_Test_Flag = 1;
      Test_Flag = 1;
      lcd.fillScreen(TFT_BLACK);
      Serial.println("进入测试程序");
      WiFi.disconnect();
      break;
    }
  }

  //*********************************测试程序***************************************
  //*********************************测试程序***************************************
  //*********************************测试程序***************************************
  while (Test_Flag == 1)
  {
    Ce_shi();
  }
}

//触摸Label控件
void label_xy()
{
  ui_Label = lv_label_create(ui_TOUCH);
  lv_obj_enable_style_refresh(true);
  lv_obj_set_width(ui_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Label, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_Label, -30);
  lv_obj_set_y(ui_Label, -35);
  lv_obj_set_align(ui_Label, LV_ALIGN_CENTER);
  lv_obj_set_style_text_color(ui_Label, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Label3 = lv_label_create(ui_TOUCH);
  lv_obj_enable_style_refresh(true);
  lv_obj_set_width(ui_Label3, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Label3, LV_SIZE_CONTENT);    /// 1
  lv_obj_set_x(ui_Label3, 58);
  lv_obj_set_y(ui_Label3, -35);
  lv_obj_set_align(ui_Label3, LV_ALIGN_CENTER);
  lv_obj_set_style_text_color(ui_Label3, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label3, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
}


//进度条控件
void lv_example_bar(void)
{
  //////////////////////////////
  bar = lv_bar_create(ui_MENU);
  lv_bar_set_value(bar, 0, LV_ANIM_OFF);
  lv_obj_set_width(bar, 150);
  lv_obj_set_height(bar, 15);
  lv_obj_set_x(bar, 0);
  lv_obj_set_y(bar, 90);
  lv_obj_set_align(bar, LV_ALIGN_CENTER);
  lv_obj_set_style_bg_img_src(bar, &ui_img_bar_320_01_png, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_img_src(bar, &ui_img_bar_320_02_png, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(bar, lv_color_hex(0x2D8812), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  //////////////////////
  ui_Labe2 = lv_label_create(bar);//创建标签
  lv_obj_set_style_text_color(ui_Labe2, lv_color_hex(0x09BEFB), LV_STATE_DEFAULT);
  lv_label_set_text(ui_Labe2, "0%");
  lv_obj_center(ui_Labe2);
}


void Ce_shi() {
  char serialData = Serial.read();
  switch (serialData) {
    case 'R':
      Close_Flag = 1;
      //      WiFi.disconnect();//断开连接
      lcd.fillScreen(TFT_BLACK);
      Serial.println("红屏");
      while (Close_Flag == 1)
      {
        lcd.fillScreen(TFT_RED);
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          lcd.fillScreen(TFT_BLACK);
          Serial.println("退出红屏");
          break;
        }
      }
      break;

    case 'G':
      Close_Flag = 1;
      lcd.fillScreen(TFT_BLACK);
      Serial.println("绿屏");
      while (Close_Flag == 1)
      {
        lcd.fillScreen(TFT_GREEN);
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          lcd.fillScreen(TFT_BLACK);
          Serial.println("退出绿屏");
          break;
        }
      }
      break;

    case 'B':
      Close_Flag = 1;
      lcd.fillScreen(TFT_BLACK);
      Serial.println("蓝屏");
      while (Close_Flag == 1)
      {
        lcd.fillScreen(TFT_BLUE);
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          lcd.fillScreen(TFT_BLACK);
          Serial.println("退出蓝屏");
          break;
        }
      }
      break;

    case 'S':
      Close_Flag = 1;
      lcd.fillScreen(TFT_BLACK);
      lcd.setTextFont(1);
      SPI.end();
      Serial.println("TF卡初始化");
      SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
      SD_init();
      while (Close_Flag == 1)
      {
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          lcd.fillScreen(TFT_BLACK);
          Serial.println("退出TF卡初始化");
          SPI.end();
          SPI.begin(14, 12, 13, 33);//触摸引脚
          break;
        }
      }
      break;

    case 'T':
      Close_Flag = 1;
      lcd.fillScreen(TFT_BLACK);
      Serial.println("触摸屏");
      delay(100);
      touch_flag = 0;
      lcd.setCursor(70, 70, 4);
      lcd.printf("Please Touch");
      Serial.println("TOUCH");
      delay(100);
      while (Close_Flag == 1)
      {
        uint16_t x, y;
        if (touch_flag < 3)
        {
          if (lcd.getTouch(&x, &y))
          {
            if (x > 0 && x < 60 && y > 0 && y < 60 && touch_flag == 0)
            {
              lcd.setCursor(140, 120, 4);
              lcd.printf("               ");
              lcd.setCursor(140, 140, 4);
              lcd.printf("               ");
              lcd.setCursor(120, 120, 4);
              lcd.printf("x: %i     ", x);
              lcd.setCursor(120, 140, 4);
              lcd.printf("y: %i    ", y);
              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
              touch_flag = 1;
            }
            if (x > 265 && x < 320 && y > 180 && y < 240 && touch_flag == 1)
            {
              lcd.setCursor(140, 120, 4);
              lcd.printf("               ");
              lcd.setCursor(140, 140, 4);
              lcd.printf("               ");
              lcd.setCursor(120, 120, 4);
              lcd.printf("x: %i     ", x);
              lcd.setCursor(120, 140, 4);
              lcd.printf("y: %i    ", y);
              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
              touch_flag = 3;
              Serial.println("OVER");
              delay(100);
              Serial.println("OVER");
            }
            if (x > 0 && x < 4000 && y > 0 && y < 4000)
            {
              lcd.setCursor(140, 120, 4);
              lcd.printf("               ");
              lcd.setCursor(140, 140, 4);
              lcd.printf("               ");
              lcd.setCursor(120, 120, 4);
              lcd.printf("x: %i     ", x);
              lcd.setCursor(120, 140, 4);
              lcd.printf("y: %i    ", y);
              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
            }
            CloseData = Serial.read();
            if (CloseData == 'I')  //Exit
            {
              Close_Flag = 0;
              touch_flag = 0;
              lcd.fillScreen(TFT_BLACK);
              break;
            }
            delay(80);
          }
        }
        if (touch_flag == 3)
        {
          Serial.println("OVER");
          if (lcd.getTouch(&x, &y))
          {
            lcd.fillScreen(TFT_BLACK);
            if (x > 0 && x < 4000 && y > 0 && y < 4000)
            {
              lcd.setCursor(140, 120, 4);
              lcd.printf("               ");
              lcd.setCursor(140, 140, 4);
              lcd.printf("               ");
              lcd.setCursor(120, 120, 4);
              lcd.printf("x: %i     ", x);
              lcd.setCursor(120, 140, 4);
              lcd.printf("y: %i    ", y);
              Serial.print( "Data x " );
              Serial.println( x );
              Serial.print( "Data y " );
              Serial.println( y );
            }
          }
          delay(80);
        }
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          lcd.fillScreen(TFT_BLACK);
          Serial.println("退出触摸屏");
          break;
        }
      }
      break;

    case 'L':
      Close_Flag = 1;
      Serial.println("IO口测试");
      lcd.fillScreen(TFT_BLACK);
      lcd.setCursor(50, 100, 4);
      lcd.printf("IO port output testing");
      while (Close_Flag == 1)
      {
        digitalWrite(25, HIGH);//拉高电平开灯
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          digitalWrite(25, LOW);
          lcd.fillScreen(TFT_BLACK);
          Serial.println("退出IO口测试");
          break;
        }
      }
      break;

    case 'i':
      Close_Flag = 1;
      u8g2.begin();
      u8g2.enableUTF8Print();        // enable UTF8 support for the Arduino print() function
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.setFontDirection(0);
      Serial.println("IIC");
      lcd.fillScreen(TFT_BLACK);
      lcd.setCursor(100, 100, 4);
      lcd.printf("IIC testing");
      while (Close_Flag == 1)
      {
        for (int i = 128; i > -90; i -= 20)
        {
          u8g2.firstPage();
          do {
            u8g2.drawStr(i, 25, "ELECROW");
            delay(2);
            CloseData = Serial.read();
            if (CloseData == 'I')  //Exit
            {
              Close_Flag = 0;
              lcd.fillScreen(TFT_BLACK);
              Serial.println("退出IIC");
              break;
            }
          } while ( u8g2.nextPage() );
        }
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          lcd.fillScreen(TFT_BLACK);
          Serial.println("退出IIC");
          break;
        }
      }
      break;

    case 'W':
      Close_Flag = 1;
      Serial.println("Wifi function testing");
      lcd.fillScreen(TFT_BLACK);
      lcd.setCursor(100, 100, 4);
      lcd.printf("WiFi testing");
      wifi_flag = 0;
      while (Close_Flag == 1)
      {
        if(Serial.available())
        {
          delay(200);
        }
        while (Serial.available() && wifi_flag == 0)
        {
          CloseData = Serial.read();
          buf[bufindex] = CloseData;
          bufindex+=1;
          if (CloseData == '\n')
          {
            buf[bufindex] = '\0';
            info[0] = buf;
            info[0] = strtok_r(info[0], "@", &info[1]);
            info[1] = strtok_r(info[1], "@", &info[2]);
            //sscanf(buf, "@%s@%s\n", wifi_account, wifi_password);
            delay(5);
            wifi_flag = 1;
          }
          if (bufindex >= 128)
          {
            bufindex = 0;
          }
        }
        if (wifi_flag == 1)
        {
          Serial.print("SSID: ");
          Serial.println(info[0]);
          
          Serial.print("Password: ");
          Serial.println(info[1]);
          WiFi.disconnect();
          delay(100);
          WiFi.begin(info[0], info[1]);
          WiFi.setAutoReconnect(true);
          delay(100);
          Serial.println("begining");
          while (WiFi.status() != WL_CONNECTED)
          {
            delay(100);
            CloseData = Serial.read();
            if (CloseData == 'I')  //Exit
            {
              memset(buf, 0, 128);
              bufindex = 0;
              break;
            }
            Serial.println("connecting");
          }
          Serial.println("WiFi is connected.");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
          //          lcd.fillScreen(TFT_BLACK);
          //          lcd.setCursor(80, 100, 4);
          //          lcd.printf("Wifi is connected");
          WiFi.disconnect();
          WiFi.mode(WIFI_OFF);
          wifi_flag = 2;
          memset(buf, 0, 128);
          bufindex = 0;
          delay(2500);
          ESP.restart();

        }
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          wifi_flag = 0;
          Close_Flag = 0;
          memset(buf, 0, 128);
          bufindex = 0;
          WiFi.disconnect();
          lcd.fillScreen(TFT_BLACK);
          Serial.println("Exit Wifi function testing");
          break;
        }
      }
      break;

    case 'U':
      Close_Flag = 1;
      Serial.println("打开UART1");
      lcd.fillScreen(TFT_BLACK);
      lcd.setCursor(100, 100, 4);
      lcd.printf("UART1 testing");
      while (Close_Flag == 1)
      {
        if (Serial2.available())              // if date is comming from softwareserial port ==> data is comming from SoftSerial shield
        {
          while (Serial2.available())         // reading data into char array
          {
            buffer[count++] = Serial2.read();   // writing data into array
            if (count == 256)break;
            CloseData = Serial.read();
            if (CloseData == 'I')  //Exit
            {
              break;
            }
          }
          Serial.write(buffer, count);           // if no data transmission ends, write buffer to hardware serial port
          clearBufferArray();              // call clearBufferArray function to clear the storaged data from the array
          count = 0;                       // set counter of while loop to zero
        }
        if (Serial.available())            // if data is available on hardwareserial port ==> data is comming from PC or notebook
          Serial2.write(Serial.read());       // write it to the SoftSerial shield
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          //          Serial2.println("AT+RST");
          Close_Flag = 0;
          lcd.fillScreen(TFT_BLACK);
          Serial.println("关闭UART1");
          break;
        }
      }
      break;

    case 'P':
      Close_Flag = 1;
      Serial.println("打开SPEAK");
      lcd.fillScreen(TFT_BLACK);
      lcd.setCursor(100, 100, 4);
      lcd.printf("SPEAK testing");
      Sequence.AddPlayItem(&ForceWithYou);      // Add the first sound item, this will play first
      Sequence.AddPlayItem(&Music);             // Add the music score, this will play after the first item
      DacAudio.Play(&Sequence);                 // Play the sequence, will play just the once and then stop
      while (Close_Flag == 1)
      {
        DacAudio.FillBuffer();//播放
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          DacAudio.StopAllSounds();
          delay(50);
          Close_Flag = 0;
          //          digitalWrite(25, LOW);
          lcd.fillScreen(TFT_BLACK);
          Serial.println("关闭SPEAK");
          break;
        }
      }
      break;

    case 'V':
      Close_Flag = 1;
      Serial.println("Bluetooth function");
      //BLE
      BLEDevice::init(bleServerName);  //创建BLE并设置名称
      pServer = BLEDevice::createServer();  //创建BLE服务器
      pServer->setCallbacks(new MyServerCallbacks());  //设置连接和断开调用类
      pService = pServer->createService(SERVICE_UUID); //创建BLE服务
    
      pCharacteristic = pService->createCharacteristic(  //Create ble feature（Characterristic_UUID）
                          CHARACTERISTIC_UUID,
                          BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
      pCharacteristic->setValue("ELECROW");
      //开始广播
      pAdvertising = BLEDevice::getAdvertising();  //A bleadvertising class pointer padvertising is defined, which points to bledevice:: getadvertising()
      pAdvertising->addServiceUUID(SERVICE_UUID);
  
      pAdvertising->start();  //Start broadcasting
      pService->start();
      lcd.fillScreen(TFT_BLACK);
      while (Close_Flag == 1)
      {
        if (connected_state == true) //有设备连接
        {
          lcd.fillScreen(TFT_BLACK);
          lcd.setCursor(0, 100, 4);
          lcd.printf("Bluetooth is connected");
        }
        else
        {
          lcd.fillScreen(TFT_BLACK);
          lcd.setCursor(0, 100, 4);
          lcd.printf("Bluetooth is not connected");
        }
        delay(100);
        CloseData = Serial.read();
        if (CloseData == 'I')  //Exit
        {
          Close_Flag = 0;
          pAdvertising->stop();  //停止广播
          pService->stop();
          delay(50);
          
          BLEDevice::stopAdvertising();
          BLEDevice::deinit();
          pService->executeDelete();
          
          lcd.fillScreen(TFT_BLACK);
          Serial.println("Turn off Bluetooth function");
          break;
        }
      }
      break;

    //Exit软件测试程序
    case 'C':
      NO_Test_Flag = 0;
      Test_Flag = 0;
      Wire.end();
      lcd.fillScreen(TFT_BLACK);
      Serial.println("Exit the software test program");
      delay(500);
      break;
  }
}

//SD卡初始化
int SD_init()
{

  if (!SD.begin(SD_CS))
  {
    Serial.println("Card Mount Failed");
    return 1;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No TF card attached");
    return 1;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("TF Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 2);

  //  listDir(SD, "/", 0);
  //  createDir(SD, "/mydir");
  //  listDir(SD, "/", 0);
  //  removeDir(SD, "/mydir");
  //  listDir(SD, "/", 2);
  //  writeFile(SD, "/hello.txt", "Hello ");
  //  appendFile(SD, "/hello.txt", "World!\n");
  //  readFile(SD, "/hello.txt");
  //  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  //  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  //  Serial.println("SD init over.");

  return 0;
}

//遍历SD卡目录
void listDir(fs::FS & fs, const char *dirname, uint8_t levels)
{
  //  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    //Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  i = 0;
  while (file)
  {
    if (file.isDirectory())
    {
      //      Serial.print("  DIR : ");
      //      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("FILE: ");
      Serial.print(file.name());
      lcd.setCursor(0, 2 * i);
      lcd.printf("FILE:%s", file.name());

      Serial.print("SIZE: ");
      Serial.println(file.size());
      lcd.setCursor(180, 2 * i);
      lcd.printf("SIZE:%d", file.size());
      i += 16;
    }

    file = root.openNextFile();
  }
}

void touch_calibrate()//屏幕校准
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("屏幕校准");

  //校准
  //  lcd.fillScreen(TFT_BLACK);
  //  lcd.setCursor(20, 0);
  //  Serial.println("setCursor");
  //  lcd.setTextFont(2);
  //  Serial.println("setTextFont");
  //  lcd.setTextSize(1);
  //  Serial.println("setTextSize");
  //  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  //  lcd.println("按指示触摸角落");
  Serial.println("按指示触摸角落");
  lv_timer_handler();

  Serial.println("setTextFont(1)");
  lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println(); Serial.println();
  Serial.println("//在setup()中使用此校准代码:");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");



  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();
  lcd.fillScreen(TFT_BLACK);
  //
  //  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  //  lcd.println("XZ OK!");
  //  lcd.println("Calibration code sent to Serial port.");

}
