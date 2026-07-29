#include <U8g2lib.h>
#include <Wire.h>

/*---------------------------------------------------------------
 * OLED hardware configuration
 * The SSD1306 display uses a software I2C bus on GPIO22 and GPIO21.
 *--------------------------------------------------------------*/
#define I2C_SDA 22
#define I2C_SCL 21

// Provides drawing and page-buffer control for the 128 x 64 OLED.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/I2C_SCL, /*data=*/I2C_SDA, /*reset=*/U8X8_PIN_NONE);

/**
 * @brief Initialize the OLED and scroll the ELECROW text across it.
 *
 * The Arduino runtime calls this function once after startup or reset. The
 * page loop is required by U8g2 so that every part of the frame is rendered.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);

  /*---------------------------------------------------------------
   * Initialize and configure text rendering
   * The selected font and direction remain active for every frame.
   *--------------------------------------------------------------*/
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setFontDirection(0);

  /*---------------------------------------------------------------
   * Animate the title
   * Moving the x-coordinate from right to left creates the scrolling effect.
   *--------------------------------------------------------------*/
  for (int i = 128; i > -78; i -= 20) {
    u8g2.firstPage();
    do {
      u8g2.drawStr(i, 25, "ELECROW");
      delay(2);
    } while (u8g2.nextPage());
  }
}

/**
 * @brief Leave the final OLED frame unchanged.
 *
 * The Arduino runtime calls this function repeatedly after setup(). No work is
 * required because the complete animation runs once during setup().
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}
