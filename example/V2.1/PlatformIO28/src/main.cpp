#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <DHT20.h>

// Loads the SquareLine Studio screen and its generated resources.
#include "ui.h"

/*---------------------------------------------------------------
 * Application state
 * UI button callbacks update this value; loop() applies it to GPIO25.
 *--------------------------------------------------------------*/
int led;

/*---------------------------------------------------------------
 * LVGL display configuration
 * The project uses a 320 x 240 landscape display and a partial buffer.
 *--------------------------------------------------------------*/
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

// Retains the original buffer allocation used by this PlatformIO project.
static lv_color_t buf1[screenWidth * 8];

// Provides LCD and resistive-touch access through TFT_eSPI.
TFT_eSPI lcd = TFT_eSPI();

// Provides temperature and humidity measurements over I2C.
DHT20 dht20;

// Maps raw touch readings to the 2.8-inch panel coordinates.
uint16_t calData[5] = {557, 3263, 369, 3493, 3};

/**
 * @brief Supply LVGL with the Arduino millisecond time base.
 *
 * LVGL calls this callback whenever it needs to advance timers. PlatformIO
 * registers it once from setup() before creating the display.
 *
 * @param None.
 * @return Current time in milliseconds since boot.
 */
static uint32_t lv_tick_get_ms() {
    return millis();
}

/**
 * @brief Transfer an LVGL-rendered area to the LCD.
 *
 * LVGL calls this callback after rendering a region into buf1. The final
 * flush-ready notification allows LVGL to reuse the buffer for the next area.
 *
 * @param disp LVGL display that requested the transfer.
 * @param area Inclusive rectangle to update.
 * @param px_map Pixel data for the rectangle.
 * @return Nothing.
 */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.pushColors(reinterpret_cast<uint16_t *>(px_map), w * h, true);
    lcd.endWrite();

    lv_display_flush_ready(disp);
}

// Stores the latest calibrated coordinates returned by TFT_eSPI.
uint16_t touchX, touchY;

/**
 * @brief Convert a panel touch into an LVGL pointer event.
 *
 * LVGL polls this callback while processing input. The x-axis mirror matches
 * the rotation selected for the 320 x 240 landscape interface.
 *
 * @param indev LVGL input device requesting data.
 * @param data Destination for pointer state and coordinates.
 * @return Nothing.
 */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev;
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

/**
 * @brief Initialize the display, touch device, sensor, and generated UI.
 *
 * PlatformIO's Arduino framework calls this function once after reset. The
 * callbacks are registered before ui_init() creates and loads Screen1.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
    Serial.begin(115200);

    /*---------------------------------------------------------------
     * Initialize application I/O and the DHT20 sensor
     * GPIO25 starts low so the controlled LED is off at boot.
     *--------------------------------------------------------------*/
    pinMode(25, OUTPUT);
    digitalWrite(25, LOW);
    Wire.begin(22, 21);
    dht20.begin();

    /*---------------------------------------------------------------
     * Initialize LVGL and LCD hardware
     * The explicit inversion setting matches the panel configuration.
     *--------------------------------------------------------------*/
    lv_init();
    lv_tick_set_cb(lv_tick_get_ms);
    lcd.begin();
    lcd.fillScreen(TFT_BLACK);
    lcd.invertDisplay(false);
    delay(300);

    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
    lcd.setRotation(1);
    lcd.setTouch(calData);

    /*---------------------------------------------------------------
     * Register LVGL display and input callbacks
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
 * @brief Update sensor labels, apply the LED state, and service LVGL.
 *
 * PlatformIO's Arduino framework calls this function repeatedly after setup().
 * The short delay keeps the UI responsive while allowing sensor labels to
 * follow the current DHT20 readings.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
    Serial.print(led);

    char DHT_buffer[12];
    int a = (int)dht20.getTemperature();
    int b = (int)dht20.getHumidity();
    snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", a);
    lv_label_set_text(ui_Label1, DHT_buffer);
    snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", b);
    lv_label_set_text(ui_Label2, DHT_buffer);

    if (led == 1) {
        digitalWrite(25, HIGH);
    }
    if (led == 0) {
        digitalWrite(25, LOW);
    }

    lv_timer_handler();
    delay(10);
}
