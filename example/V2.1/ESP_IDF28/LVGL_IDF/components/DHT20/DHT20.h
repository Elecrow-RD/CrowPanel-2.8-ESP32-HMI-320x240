#ifndef DHT20_H
#define DHT20_H

#include <Arduino.h>
#include <Wire.h>

class DHT20 {
public:
  DHT20(TwoWire *wire = &Wire, uint8_t address = 0x38);
  int begin();
  int readTempHumidity(int *temperature, int *humidity);

private:
  uint8_t writeCommand(const void *buffer, size_t size);
  bool readData(void *buffer, size_t size);

  TwoWire *_wire;
  uint8_t _address;
};

#endif
