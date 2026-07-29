#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <DHT20.h>
#include "ui.h"

/*---------------------------------------------------------------
 * LVGL display configuration
 * The landscape UI uses a partial buffer containing one eighth of the frame.
 *--------------------------------------------------------------*/
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

// Holds pixels rendered by LVGL before they are transferred to the LCD.
static lv_color_t buf1[screenWidth * screenHeight / 8];

// Provides LCD and resistive-touch access through TFT_eSPI.
TFT_eSPI lcd = TFT_eSPI();

// Provides temperature and humidity measurements over I2C.
DHT20 dht20;

// Maps raw resistive-touch readings to this panel's screen coordinates.
uint16_t calData[5] = {189, 3416, 359, 3439, 1};

// Shares the on-screen LED selection with the C-based SquareLine event code.
extern "C" int led = 0;

/**
 * @brief Transfer a rendered LVGL area to the LCD.
 *
 * LVGL calls this function whenever a region of the display buffer is ready.
 * The completion notification is essential because LVGL must not reuse the
 * buffer while the transfer is still in progress.
 *
 * @param disp LVGL display that requested the transfer.
 * @param area Inclusive screen rectangle to update.
 * @param px_map Pixel data generated for the rectangle.
 * @return Nothing.
 */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushColors((uint16_t *)px_map, w * h, true);
  lcd.endWrite();

  lv_display_flush_ready(disp);
}

// Stores the latest calibrated coordinates returned by TFT_eSPI.
uint16_t touchX, touchY;

/**
 * @brief Convert the current panel touch into an LVGL pointer event.
 *
 * LVGL calls this function while processing input. TFT_eSPI applies calData,
 * so the coordinates can be passed directly to the landscape LVGL display.
 *
 * @param indev LVGL input device requesting data.
 * @param data Destination for the pointer state and coordinates.
 * @return Nothing.
 */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  bool touched = lcd.getTouch(&touchX, &touchY, 400);
  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

/**
 * @brief Initialize the hardware and run the LVGL application task.
 *
 * ESP-IDF calls this function once after system startup. Arduino compatibility
 * is initialized first, then the function remains in its FreeRTOS loop to
 * service LVGL, sample the DHT20, and apply the UI-controlled LED state.
 *
 * @param None.
 * @return Nothing; the function runs for the lifetime of the application.
 */
extern "C" void app_main() {
  initArduino();
  Serial.begin(115200);

  /*---------------------------------------------------------------
   * Initialize application I/O and the DHT20 sensor
   * GPIO25 starts low so the controlled LED is off during startup.
   *--------------------------------------------------------------*/
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);

  Wire.begin(22, 21);
  Wire.setClock(100000);
  delay(100);
  if (dht20.begin() != 0) {
    Serial.println("DHT20 init failed");
  }

  /*---------------------------------------------------------------
   * Initialize LVGL and the LCD
   * millis() supplies LVGL's time base for input and timer processing.
   *--------------------------------------------------------------*/
  lv_init();
  lv_tick_set_cb(millis);

  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  delay(300);

  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  lcd.setRotation(1);
  lcd.setTouch(calData);

  /*---------------------------------------------------------------
   * Register display and touch callbacks
   * LVGL renders through my_disp_flush() and polls my_touchpad_read().
   *--------------------------------------------------------------*/
  lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();

  // Records the last successful sensor polling interval boundary.
  uint32_t last_sensor_ms = 0;

  while (1) {
    lv_timer_handler();

    /*---------------------------------------------------------------
     * Update sensor labels once per second
     * Limiting I2C reads keeps the faster 10 ms UI service loop responsive.
     *--------------------------------------------------------------*/
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

    // The UI callback changes led; this loop owns the physical GPIO update.
    digitalWrite(25, led ? HIGH : LOW);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
