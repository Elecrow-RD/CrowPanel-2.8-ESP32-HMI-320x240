#include "DHT20.h"

DHT20::DHT20(TwoWire *wire, uint8_t address)
    : _wire(wire), _address(address) {}

int DHT20::begin() {
  const uint8_t status_command = 0x71;
  uint8_t status = 0xff;

  delay(100);
  if (writeCommand(&status_command, 1) != 0 || !readData(&status, 1)) {
    return -1;
  }
  return (status & 0x08) != 0 ? 0 : -1;
}

int DHT20::readTempHumidity(int *temperature, int *humidity) {
  if (temperature == nullptr || humidity == nullptr) {
    return -1;
  }

  const uint8_t measure_command[] = {0xac, 0x33, 0x00};
  uint8_t data[6] = {};
  if (writeCommand(measure_command, sizeof(measure_command)) != 0) {
    return -1;
  }

  for (int retries = 20; retries > 0; --retries) {
    delay(10);
    if (!readData(data, sizeof(data))) {
      return -1;
    }
    if ((data[0] & 0x80) == 0) {
      const uint32_t temperature_raw =
          ((uint32_t)(data[3] & 0x0f) << 16) |
          ((uint32_t)data[4] << 8) | data[5];
      const uint32_t humidity_raw =
          ((uint32_t)data[1] << 12) |
          ((uint32_t)data[2] << 4) | ((data[3] & 0xf0) >> 4);

      *temperature = (int)(temperature_raw / 5242) - 50;
      *humidity = (int)((humidity_raw * 100ULL) / 0x100000);
      return 0;
    }
  }
  return -1;
}

uint8_t DHT20::writeCommand(const void *buffer, size_t size) {
  if (buffer == nullptr || size == 0) {
    return 4;
  }

  const uint8_t *bytes = static_cast<const uint8_t *>(buffer);
  _wire->beginTransmission(_address);
  _wire->write(bytes, size);
  return _wire->endTransmission();
}

bool DHT20::readData(void *buffer, size_t size) {
  if (buffer == nullptr || size == 0) {
    return false;
  }

  delay(10);
  if (_wire->requestFrom(_address, size) != size) {
    return false;
  }

  uint8_t *bytes = static_cast<uint8_t *>(buffer);
  for (size_t i = 0; i < size; ++i) {
    bytes[i] = _wire->read();
  }
  return true;
}
