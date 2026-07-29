#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <Crowbits_DHT20.h>

// Provides the SquareLine Studio screen, labels, buttons, and image resources.
#include "ui.h"

/*---------------------------------------------------------------
 * Application state
 * SquareLine button callbacks update this value to control GPIO25.
 *--------------------------------------------------------------*/
int led;

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
Crowbits_DHT20 dht20;

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

// Stores the latest raw coordinates returned by TFT_eSPI.
uint16_t touchX, touchY;

/**
 * @brief Convert the current panel touch into an LVGL pointer event.
 *
 * LVGL calls this function while processing input. The x-axis is mirrored to
 * match the landscape display orientation used by this Arduino project.
 *
 * @param indev LVGL input device requesting data.
 * @param data Destination for the pointer state and coordinates.
 * @return Nothing.
 */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  bool touched = lcd.getTouch(&touchX, &touchY, 600);
  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;

    data->point.x = screenWidth - touchX;
    data->point.y = touchY;

    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

/*---------------------------------------------------------------
 * Serial receive buffer
 * These globals are retained for the optional Serial2 data path.
 *--------------------------------------------------------------*/
// Temporarily stores bytes received over a serial port.
unsigned char buffer[256];

// Tracks how many valid bytes are currently held in buffer.
int count = 0;

/**
 * @brief Clear the portion of the serial buffer currently in use.
 *
 * Call this function after forwarding count bytes from buffer. The current
 * application does not yet invoke it, but it supports the initialized
 * Serial2 interface without changing the main UI flow.
 *
 * @param None.
 * @return Nothing.
 */
void clearBufferArray() {
  for (int i = 0; i < count; i++) {
    buffer[i] = NULL;
  }
}

/**
 * @brief Initialize serial links, sensors, LCD, touch input, and LVGL UI.
 *
 * The Arduino runtime calls this function once after startup or reset. Each
 * callback is registered before ui_init() creates and loads the screen.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);

  /*---------------------------------------------------------------
   * Initialize application I/O and the DHT20 sensor
   * GPIO25 starts low so the controlled LED is off during startup.
   *--------------------------------------------------------------*/
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);

  Wire.begin(22, 21);
  dht20.begin();

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
}

/**
 * @brief Refresh sensor labels, apply the UI LED state, and service LVGL.
 *
 * The Arduino runtime calls this function repeatedly after setup(). A short
 * delay prevents the UI loop from monopolizing the processor.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  Serial.print(led);

  /*---------------------------------------------------------------
   * Update temperature and humidity labels
   * Integer conversion matches the numeric labels designed in the UI.
   *--------------------------------------------------------------*/
  char DHT_buffer[6];
  int a = (int)dht20.getTemperature();
  int b = (int)dht20.getHumidity();
  snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", a);
  lv_label_set_text(ui_Label1, DHT_buffer);
  snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", b);
  lv_label_set_text(ui_Label2, DHT_buffer);

  /*---------------------------------------------------------------
   * Apply the state selected by the on-screen buttons
   * Separate conditions preserve the original UI-to-GPIO behavior.
   *--------------------------------------------------------------*/
  if (led == 1) {
    digitalWrite(25, HIGH);
    Serial.print("led_on");
  }

  if (led == 0) {
    digitalWrite(25, LOW);
    Serial.print("led_off");
  }

  lv_timer_handler();
  delay(10);
}
