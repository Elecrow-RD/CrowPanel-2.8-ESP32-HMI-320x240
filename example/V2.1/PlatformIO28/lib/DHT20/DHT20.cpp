#include "DHT20.h"

DHT20::DHT20(TwoWire *pWire, uint8_t address)
  : _pWire(pWire), _address(address) {
}

int DHT20::begin() {
  uint8_t readCMD[1] = {0x71};
  uint8_t data;
  delay(100);
  writeCommand(readCMD, 1);
  readData(&data, 1);
  if ((data | 0x8) == 0 || data == 255) {
    return 1;
  }
  return 0;
}

int DHT20::getTemperature() {
  uint8_t readCMD[3] = {0xac, 0x33, 0x00};
  uint8_t data[6] = {0};
  int retries = 10;
  writeCommand(readCMD, 3);
  while (retries--) {
    delay(10);
    readData(data, 6);
    if ((data[0] >> 7) == 0) {
      break;
    }
  }
  uint32_t rawData = ((data[3] & 0xf) << 16) + ((data[4] & 0xff) << 8) + data[5];
  return (int)rawData / 5242 - 50;
}

int DHT20::getHumidity() {
  uint8_t readCMD[3] = {0xac, 0x33, 0x00};
  uint8_t data[6] = {0};
  int retries = 10;
  writeCommand(readCMD, 3);
  while (retries--) {
    delay(10);
    readData(data, 6);
    if ((data[0] >> 7) == 0) {
      break;
    }
  }
  uint32_t rawData = ((data[1] & 0xff) << 12) + ((data[2] & 0xff) << 4) + ((data[3] & 0xf0) >> 4);
  return (int)(((float)rawData / 0x100000) * 100);
}

void DHT20::writeCommand(const void *pBuf, size_t size) {
  const uint8_t *data = static_cast<const uint8_t *>(pBuf);
  _pWire->beginTransmission(_address);
  for (size_t i = 0; i < size; i++) {
    _pWire->write(data[i]);
  }
  _pWire->endTransmission();
}

uint8_t DHT20::readData(void *pBuf, size_t size) {
  delay(10);
  uint8_t *data = static_cast<uint8_t *>(pBuf);
  _pWire->requestFrom(_address, size);
  for (size_t i = 0; i < size; i++) {
    data[i] = _pWire->read();
  }
  return 1;
}
