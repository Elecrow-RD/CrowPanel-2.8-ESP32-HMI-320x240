#include <TFT_eSPI.h>

/*---------------------------------------------------------------
 * Touch-screen objects and calibration
 * The stored values map raw resistive-touch readings to screen coordinates.
 *--------------------------------------------------------------*/
// Provides LCD and resistive-touch access through TFT_eSPI.
TFT_eSPI lcd = TFT_eSPI();

// Stores the latest calibrated screen coordinate.
uint16_t touchX, touchY;

// Stores the known calibration values for this 2.8-inch panel.
uint16_t calData[5] = {557, 3263, 369, 3493, 3};

/**
 * @brief Initialize the display and apply saved touch calibration data.
 *
 * The Arduino runtime calls this function once after startup or reset. Call
 * touch_calibrate() here instead when new calibration values are required.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.setRotation(1);

  // Enable this call only while collecting calibration values for a panel.
  // touch_calibrate();
  lcd.setTouch(calData);
}

/**
 * @brief Read touches and report their calibrated coordinates.
 *
 * The Arduino runtime calls this function repeatedly after setup(). A pressure
 * threshold of 600 filters weak or accidental contact with the panel.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  bool touched = lcd.getTouch(&touchX, &touchY, 600);
  if (touched) {
    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

/**
 * @brief Run the interactive four-corner touch calibration.
 *
 * This function is called manually from setup() when a panel must be
 * calibrated. It prints the resulting array for reuse with lcd.setTouch().
 *
 * @param None.
 * @return Nothing.
 */
void touch_calibrate() {
  // Receives the five values calculated by TFT_eSPI for this session.
  uint16_t calData[5];

  // Reserved by the original calibration flow; retained to preserve the code.
  uint8_t calDataOK = 0;

  Serial.println("Touch-screen calibration");
  Serial.println("Please touch the corners as directed");

  // lv_timer_handler();
  lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println(); Serial.println();
  Serial.println("//Use this calibration code in setup():");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++) {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();
}
