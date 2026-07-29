/*---------------------------------------------------------------
 * LED hardware configuration
 * GPIO25 drives the on-board LED used by this example.
 *--------------------------------------------------------------*/
#define D_PIN 25

/**
 * @brief Initialize the serial port and LED output.
 *
 * The Arduino runtime calls this function once after startup or reset.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  pinMode(D_PIN, OUTPUT);
}

/**
 * @brief Blink the LED with equal on and off times.
 *
 * The Arduino runtime calls this function repeatedly after setup(). Each
 * complete cycle lasts one second, producing a steady 1 Hz blink pattern.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  digitalWrite(D_PIN, HIGH);
  delay(500);
  digitalWrite(D_PIN, LOW);
  delay(500);
}
