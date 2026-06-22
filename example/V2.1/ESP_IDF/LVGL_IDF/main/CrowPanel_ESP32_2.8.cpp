#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <DHT20.h>
#include "ui.h"


static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 8 ];

TFT_eSPI lcd = TFT_eSPI(); 
DHT20 dht20;
uint16_t calData[5] = {189, 3416, 359, 3439, 1};

extern "C" int led = 0;


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

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
  bool touched = lcd.getTouch( &touchX, &touchY, 400);
  if ( !touched )
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

extern "C" void app_main()
{
  initArduino();
  Serial.begin(115200);

  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);

  Wire.begin(22, 21);
  Wire.setClock(100000);
  delay(100);
  if (dht20.begin() != 0) {
    Serial.println("DHT20 init failed");
  }

  lv_init();

  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  delay(300);
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  lcd.setRotation(1);
  lcd.setTouch( calData );

  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 8 );

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

  ui_init();

  uint32_t last_sensor_ms = 0;

  while (1)
  {
    lv_tick_inc(10);
    lv_timer_handler();

    uint32_t now = millis();
    if (now - last_sensor_ms >= 1000) {
      last_sensor_ms = now;
      int temperature = 0;
      int humidity = 0;
      if (dht20.readTempHumidity(&temperature, &humidity) == 0) {
        char dht_buffer[8];
        snprintf(dht_buffer, sizeof(dht_buffer), "%d", temperature);
        lv_label_set_text(ui_Label1, dht_buffer);
        snprintf(dht_buffer, sizeof(dht_buffer), "%d", humidity);
        lv_label_set_text(ui_Label2, dht_buffer);
      }
    }

    digitalWrite(25, led ? HIGH : LOW);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
