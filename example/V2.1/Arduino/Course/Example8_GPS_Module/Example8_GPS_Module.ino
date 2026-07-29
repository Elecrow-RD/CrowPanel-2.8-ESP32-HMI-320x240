/*---------------------------------------------------------------
 * GPS UART configuration
 * ESP32 UART2 receives from the GPS TX pin and transmits to the GPS RX pin.
 *--------------------------------------------------------------*/
#define GPS_RX 16
#define GPS_TX 17

// Provides the hardware UART used to communicate with the GPS module.
HardwareSerial gpsSerial(2);

// Temporarily stores bytes received from the GPS module.
unsigned char buffer[256];

// Tracks how many valid bytes are currently held in buffer.
int count = 0;

/**
 * @brief Initialize the GPS UART and USB serial monitor.
 *
 * The Arduino runtime calls this function once after startup or reset. Both
 * links use 9600 baud so NMEA data can be forwarded without conversion.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.begin(9600);
}

/**
 * @brief Bridge data between the GPS module and the serial monitor.
 *
 * The Arduino runtime calls this function repeatedly after setup(). Incoming
 * GPS data is sent to the computer in blocks, while computer input is sent
 * back one byte at a time to the module.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  if (gpsSerial.available()) {
    // Stop at the fixed buffer boundary even if more UART data is waiting.
    while (gpsSerial.available()) {
      buffer[count++] = gpsSerial.read();
      if (count == 256) break;
    }

    Serial.write(buffer, count);
    clearBufferArray();
    count = 0;
  }

  if (Serial.available())
    gpsSerial.write(Serial.read());
}

/**
 * @brief Clear the portion of the receive buffer used by the last transfer.
 *
 * loop() calls this function after forwarding a block to the serial monitor.
 *
 * @param None.
 * @return Nothing.
 */
void clearBufferArray() {
  for (int i = 0; i < count; i++) {
    buffer[i] = 0;
  }
}
