#include <lvgl.h>
#include <TFT_eSPI.h>
#include <demos/lv_demos.h>
#include<Arduino.h>
#include <Crowbits_DHT20.h>

//UI
#include "ui.h"
int led;

/*Changing the screen resolution*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 8 ];

TFT_eSPI lcd = TFT_eSPI(); /* TFT Example */
// Initialize DHT20 sensor
Crowbits_DHT20 dht20;
uint16_t calData[5] = { 189, 3416, 359, 3439, 1 };

/* Display Refresh */
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
/*Read Touchpad*/
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

    /*Setting the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print( "Data x " );
    Serial.println( touchX );

    Serial.print( "Data y " );
    Serial.println( touchY );
  }
}

void setup()
{
  Serial.begin( 115200 ); /*Initializing the Serial Port*/
  Serial2.begin( 9600 ); /*Initialize Serial Port 2*/

  //IO Port Pins
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  Wire.begin(22, 21);
  dht20.begin();

  lv_init();

  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  delay(300);
  lcd.setTouch( calData );
  //Backlight Pins
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  lcd.setRotation(1);
 
  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 8 );

  /*Initializing the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*Change the following line to display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the (virtual) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

  ui_init();
}

void loop()
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
  lv_timer_handler(); /* let the GUI do its work */
  delay( 10 );
}



