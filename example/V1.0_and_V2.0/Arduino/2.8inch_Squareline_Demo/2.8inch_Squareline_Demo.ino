#include <TFT_eSPI.h>
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <Crowbits_DHT20.h>

#define Display_28        //according to the board you using ,if you using the ESP32 Display 3.5inch board, please define 'Display_35'.if using 2.4inch board,please define 'Display_24'.

#define LED 32

#define WIFI_SSID "yanfa_software"
#define WIFI_PASSWORD "yanfa-123456"

// Raspberry Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 50, 233)

// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1885

// Temperature MQTT Topics
#define MQTT_PUB_LED_S  "esp32/led/state"
#define MQTT_PUB_LED_C "esp32/led/command"
#define MQTT_PUB_TEMP "esp32/temperature"
#define MQTT_PUB_HUM  "esp32/humidity"
#define mqtt_username  "elecrow"
#define mqtt_password  "elecrow2014"

// Initialize DHT20 sensor
Crowbits_DHT20 dht20;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

#include <lvgl.h>
#include "ui.h"


#if defined Display_35      //ESP32 Display 3.5inch Board
/*screen resolution*/
static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;
uint16_t calData[5] = { 353, 3568, 269, 3491, 7  };     /*touch caldata*/

#elif defined Display_24    //ESP32 Display 2.4inch Board
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
uint16_t calData[5] = { 557, 3263, 369, 3493, 3  };

#elif defined Display_28    //ESP32 Display 2.8inch Board
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
uint16_t calData[5] = { 189, 3416, 359, 3439, 1 };
#endif


TFT_eSPI lcd = TFT_eSPI(); /* TFT entity */


static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 13 ];


int led_flag = 0;
int led_flag_Lock = 0;
int  temp;
int  hum;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

//_______________________
/* display flash */
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
/*touch read*/
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

    /*set location*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print( "Data x " );
    Serial.println( touchX );

    Serial.print( "Data y " );
    Serial.println( touchY );
  }
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe(MQTT_PUB_LED_C, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  Serial.println("Publish received.");
  for (int i = 0; i < len; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println("");
  if (strncmp(payload, "ON", 2) == 0) {
    digitalWrite(LED, HIGH);
    mqttClient.publish(MQTT_PUB_LED_S, 0, true, "ON");
    //    lv_event_send(ui_Switch2, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);
    lv_label_set_text(ui_Label1, "ON");
  }
  if (strncmp(payload, "OFF", 3) == 0) {
    digitalWrite(LED, LOW);
    mqttClient.publish(MQTT_PUB_LED_S, 0, true, "OFF");
    lv_obj_clear_state(ui_Switch2, LV_STATE_CHECKED);
    lv_label_set_text(ui_Label1, "OFF");
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(9600); /*serial init */
  Wire.begin(22, 21);
  Serial.println();
  dht20.begin();
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  mqttClient.setCredentials(mqtt_username, mqtt_password);
  connectToWifi();
  //LCD init
  lcd.begin();
  lcd.setRotation(1);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTouch(calData);
  delay(100);
  //background light pin
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);


  //lvgl init
  lv_init();

  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 13 );

  /*Display init*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*Display driver port of LVGL*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*touch driver port of LVGL*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

  ui_init();        //LVGL UI init
  Serial.println( "Setup done" );
}

char buffer_t[10];
char buffer_h[10];

void loop()
{
  lv_timer_handler();
  delay(5);
  if (led_flag_Lock == 1)
  {
    if (led_flag == 1)
    {
      led_flag_Lock = 0;
      digitalWrite(LED, HIGH);
      mqttClient.publish(MQTT_PUB_LED_S, 0, true, "ON");
      //        lv_switch_on(ui_Switch2, LV_ANIM_ON);
    }
    else
    {
      led_flag_Lock = 0;
      digitalWrite(LED, LOW);
      mqttClient.publish(MQTT_PUB_LED_S, 0, true, "OFF");
      //        lv_switch_off(ui_Switch2, LV_ANIM_ON);
    }
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    // New DHT sensor readings
    hum = (int)dht20.getHumidity();
    // Read temperature as Celsius (the default)
    temp = (int)dht20.getTemperature();
    Serial.print("hum:");
    Serial.println( hum);
    Serial.print("temp:");
    Serial.println( temp);
    itoa(hum, buffer_h, 10);
    itoa(temp, buffer_t, 10);
    lv_label_set_text(ui_Label3, buffer_t);
    lv_label_set_text(ui_Label5, buffer_h);
    if (isnan(temp) || isnan(hum))
    {
      Serial.println(F("Failed to read from DHT20 sensor!"));
      return;
    }

    // Publish an MQTT message on topic esp32/dht20/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEMP, packetIdPub1);
    Serial.printf("Message: %.2f \n", temp);
    // Publish an MQTT message on topic esp32/dht20/humidity
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub2);
    Serial.printf("Message: %.2f \n", hum);
  }
}
