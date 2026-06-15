#include "Arduino.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <examples/lv_examples.h>
#include <demos/lv_demos.h>

/*更改屏幕分辨率*/
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[screenWidth * screenHeight / 8];

TFT_eSPI lcd = TFT_eSPI(); /* TFT实例 */
//UI
#include "ui.h"
#include <Ticker.h> //Call the ticker. H Library
Ticker ticker1;
static int first_flag = 0;
extern int zero_clean;
extern int goto_widget_flag;
extern int bar_flag;
extern lv_obj_t *ui_MENU;
extern lv_obj_t *ui_TOUCH;
extern lv_obj_t *ui_JIAOZHUN;
extern lv_obj_t *ui_Label2;
static lv_obj_t *ui_Label;  //TOUCH界面label
static lv_obj_t *ui_Label3; //TOUCH界面label3
static lv_obj_t *ui_Labe2;  //Menu界面进度条label
static lv_obj_t *bar;       //Menu界面进度条
static int val = 100;

/* 显示器刷新 */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
  lcd.endWrite();

  lv_disp_flush_ready(disp);
}

uint16_t touchX, touchY;
/*读取触摸板*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  bool touched = lcd.getTouch(&touchX, &touchY, 600);
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*设置坐标*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

char CloseData;
int NO_Test_Flag = 0;
int Test_Flag = 0;
int Close_Flag = 0;
uint16_t calData[5] = {189, 3416, 359, 3439, 1};
void callback1() //Callback function
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
      bar_flag = 0;         //停止进度条标志
      goto_widget_flag = 1; //进入widget标志
    }
  }
}

//触摸Label控件
void label_xy()
{
  ui_Label = lv_label_create(ui_TOUCH);
  lv_obj_enable_style_refresh(true);
  lv_obj_set_width(ui_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_Label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_Label, -30);
  lv_obj_set_y(ui_Label, -35);
  lv_obj_set_align(ui_Label, LV_ALIGN_CENTER);
  lv_obj_set_style_text_color(ui_Label, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Label3 = lv_label_create(ui_TOUCH);
  lv_obj_enable_style_refresh(true);
  lv_obj_set_width(ui_Label3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_Label3, LV_SIZE_CONTENT); /// 1
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
  ui_Labe2 = lv_label_create(bar); //创建标签
  lv_obj_set_style_text_color(ui_Labe2, lv_color_hex(0x09BEFB), LV_STATE_DEFAULT);
  lv_label_set_text(ui_Labe2, "0%");
  lv_obj_center(ui_Labe2);
}

void touch_calibrate() //屏幕校准
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("屏幕校准");

  Serial.println("按指示触摸角落");
  lv_timer_handler();

  Serial.println("setTextFont(1)");
  lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println();
  Serial.println();
  Serial.println("//在setup()中使用此校准代码:");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4)
      Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println();
  Serial.println();
  lcd.fillScreen(TFT_BLACK);
}

extern "C" void app_main()
{
  Serial.begin(115200);
  //IO口引脚
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);

  //lvgl初始化
  lv_init();

  //LCD初始化
  lcd.begin(); /*初始化*/
  lcd.fillScreen(TFT_BLACK);
  delay(300);
  //背光引脚
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  lcd.setRotation(1); /* 旋转 */

  //校准模式。一是四角定位、二是直接输入模拟数值直接定位
  //屏幕校准
  //  touch_calibrate();
  lcd.setTouch(calData);

  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, screenWidth * screenHeight / 8);

  /*初始化显示*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*将以下行更改为显示分辨率*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*初始化（虚拟）输入设备驱动程序*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  // lcd.fillScreen(TFT_RED);
  // delay(1000);
  // lcd.fillScreen(TFT_GREEN);
  // delay(1000);
  // lcd.fillScreen(TFT_BLUE);
  // delay(1000);
  ui_init(); //开机UI界面
  while (1)
  {
    if (goto_widget_flag == 1) //进入widget
    {
      if (ticker1.active() == true)
      {
        ticker1.detach();
      }
      goto_widget_flag = 0;
      delay(300);
      break;
    }

    if (goto_widget_flag == 3) //进入触摸界面，先把进度条线程关闭
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

    if (goto_widget_flag == 4) //触摸界面返回到Menu界面,使进度条加满
    {
      val = 100;
      delay(100);
      ticker1.attach_ms(30, callback1); //每35ms调用callback1
      goto_widget_flag = 0;
    }

    if (goto_widget_flag == 5) //触发校准信号
    {
      lv_scr_load_anim(ui_touch_calibrate, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      lv_timer_handler();
      lv_timer_handler();
      delay(100);
      touch_calibrate(); //触摸校准
      lcd.setTouch(calData);
      lv_scr_load_anim(ui_TOUCH, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      lv_timer_handler();
      delay(100);
      goto_widget_flag = 3; //进入触摸界面标志
      touchX = 0;
      touchY = 0;
    }

    if (bar_flag == 6) //刚开机进入Menu界面时运行进度条一次，之后就不再运行
    {
      if (first_flag == 0)
      {
        lv_example_bar();
        ticker1.attach_ms(30, callback1); //每35ms调用callback1
        first_flag = 1;
      }
    }

    lv_timer_handler();
  }

  lcd.fillScreen(TFT_BLACK);
  lv_demo_widgets(); //主UI界面
  Serial.println("Setup done");
  while (1)
  {
    lv_timer_handler();
    delay(5);
  }
}
