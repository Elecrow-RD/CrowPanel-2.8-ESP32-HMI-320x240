// ESP32 GPS Module Example
// Original code based on SoftwareSerial, ESP32 uses HardwareSerial instead

// Remove SoftwareSerial.h, not needed for ESP32

#define GPS_RX 16   // GPS Module TX -> ESP32 GPIO16
#define GPS_TX 17   // GPS Module RX -> ESP32 GPIO17

HardwareSerial gpsSerial(2);  // Use ESP32 UART2

unsigned char buffer[256]; // buffer array for data receive over serial port
int count = 0;     // counter for buffer array 

void setup()
{
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);  // GPS serial, 9600 bps 8-N-1
  Serial.begin(9600);                                   // Debug serial (USB)
}

void loop()
{
  if (gpsSerial.available())              // if data is coming from GPS module
  {
    while (gpsSerial.available())         // read data into buffer
    {
      buffer[count++] = gpsSerial.read(); // write to array
      if (count == 256) break;
    }
    Serial.write(buffer, count);          // output to serial monitor
    clearBufferArray();                   // clear buffer
    count = 0;                            // reset counter
  }

  if (Serial.available())                 // if data is coming from PC
    gpsSerial.write(Serial.read());       // forward to GPS module
}

void clearBufferArray()                   // function to clear buffer array
{
  for (int i = 0; i < count; i++)
  {
    buffer[i] = 0;                        // clear with 0 (more standard than NULL on ESP32)
  }
}