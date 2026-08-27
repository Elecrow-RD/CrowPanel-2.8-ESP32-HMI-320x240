#ifndef DHT20_H
#define DHT20_H

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief Minimal I2C driver for the DHT20 temperature and humidity sensor.
 *
 * The PlatformIO lesson uses this small local library so a clean checkout does
 * not depend on an unpinned copy hidden inside `.pio/libdeps`.
 */
class DHT20 {
public:
  /**
   * @brief Construct a sensor interface.
   * @param pWire I2C bus object; Wire is used by default.
   * @param address Sensor I2C address; 0x38 is the DHT20 default.
   */
  DHT20(TwoWire *pWire = &Wire, uint8_t address = 0x38);

  /**
   * @brief Probe the sensor and prepare it for measurements.
   * @return 0 when the sensor responds, otherwise a non-zero error code.
   */
  int begin(void);

  /**
   * @brief Read ambient temperature.
   * @return Temperature in degrees Celsius.
   */
  int getTemperature();

  /**
   * @brief Read relative humidity.
   * @return Relative humidity in percent.
   */
  int getHumidity();

private:
  void writeCommand(const void *pBuf, size_t size);
  uint8_t readData(void *pBuf, size_t size);

  TwoWire *_pWire;
  uint8_t _address;
};

#endif
