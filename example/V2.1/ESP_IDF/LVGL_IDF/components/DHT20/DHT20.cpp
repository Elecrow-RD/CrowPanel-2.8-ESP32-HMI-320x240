#include <DHT20.h>


DHT20::DHT20(TwoWire * pWire,uint8_t address)
  : _pWire(pWire) {
  _address = address;
}

int DHT20::begin() {
  uint8_t readCMD[3]={0x71};
  uint8_t data;
  delay(100);
  //_pWire->begin(14, 12);
  //check if the IIC communication works 
  writeCommand(readCMD,1);
  
  readData(&data, 1);
  //Serial.println(data);
  if((data | 0x8) == 0){
     return 1;
  }
  if(data == 255) return 1;
  return 0;
}

int DHT20::getTemperature() {
  uint8_t readCMD[3]={0xac,0x33,0x00};
  uint8_t data[6] = {0};
  int retries = 10;
  int temperature;
  // when the returned data is wrong, request to get data again until the data is correct. 
  writeCommand(readCMD, 3);
  while(retries--) {
    delay(10);
    readData(data,6);
    if((data[0] >> 7) == 0){
      // DBG("bus not busy");
       break;
    }
  }
  uint32_t temp = data[3] & 0xff;
  uint32_t temp1 = data[4] & 0xff;
  uint32_t rawData = 0;
  rawData = ((temp&0xf)<<16)+(temp1<<8)+(data[5]);
  //DBG(rawData);
  //DBG((temp&0xf)<<16);
  temperature = (int)rawData/5242 -50;
  //DBG(temperature)
  return temperature;
}

int DHT20::getHumidity() {
  uint8_t readCMD[3]={0xac,0x33,0x00};
  uint8_t data[6] = {0};
  int retries = 10;
  float humidity_f;
  int humidity_i;
  // when the returned data is wrong, request to get data again until the data is correct. 
  writeCommand(readCMD, 3);
  while(retries--) {
    delay(10);
    readData(data,6);
    if((data[0] >> 7) == 0){
       //DBG("bus not busy");
       break;
    }
  }
  uint32_t temp = data[1] & 0xff;
  uint32_t temp1 = data[2] & 0xff;
  uint32_t rawData = 0;
  rawData = (temp<<12)+(temp1<<4)+((data[3]&0xf0)>>4);
  //DBG(rawData);
  //DBG(temp<<12);
  humidity_f = (float)rawData/0x100000;
  humidity_i = (int)(humidity_f*100);
  //DBG(humidity)
  return humidity_i;
}

int DHT20::readTempHumidity(int *temperature, int *humidity) {
  if (temperature == NULL || humidity == NULL) {
    return -1;
  }

  uint8_t readCMD[3] = {0xac, 0x33, 0x00};
  uint8_t data[6] = {0};
  int retries = 20;

  writeCommand(readCMD, 3);
  while (retries--) {
    delay(10);
    readData(data, 6);
    if ((data[0] >> 7) == 0) {
      break;
    }
  }

  if ((data[0] >> 7) != 0) {
    return -1;
  }

  uint32_t temp_raw = ((uint32_t)(data[3] & 0x0f) << 16) + ((uint32_t)data[4] << 8) + data[5];
  uint32_t hum_raw = ((uint32_t)data[1] << 12) + ((uint32_t)data[2] << 4) + ((data[3] & 0xf0) >> 4);

  *temperature = (int)(temp_raw / 5242) - 50;
  *humidity = (int)(((float)hum_raw / 0x100000) * 100.0f);
  return 0;
}

void DHT20::writeCommand(const void *pBuf, size_t size) {
  if (pBuf == NULL) {
   // DBG("pBuf ERROR!! : null pointer");
  }

  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pWire->beginTransmission(_address);
  for (uint8_t i = 0; i < size; i++) {
    
    _pWire->write(_pBuf[i]);
    
  }
  _pWire->endTransmission();
}

uint8_t DHT20::readData(void *pBuf, size_t size) {
  delay(10);
  if (pBuf == NULL) {
    return 0;
  }

  uint8_t * _pBuf = (uint8_t *)pBuf;
  size_t bytes_read = _pWire->requestFrom(_address, (uint8_t)size);
  if (bytes_read < size) {
    return 0;
  }

  for (uint8_t i = 0; i < size; i++) {
    _pBuf[i] = _pWire->read();
  }
  return 1;
}