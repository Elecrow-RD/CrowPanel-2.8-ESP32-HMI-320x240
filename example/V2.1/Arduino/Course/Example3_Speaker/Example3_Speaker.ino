#include <driver/dac.h>

#define SPEAKER_PIN 26  // GPIO26 = DAC_CHANNEL_2

void setup() {
  Serial.begin(115200);
  
  // Initialize DAC channel 2 (GPIO26)
  dac_output_enable(DAC_CHANNEL_2);
}

// 256-point 8-bit sine wave lookup table (value range 0-255)
const uint8_t sineWave[256] = {
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173,
  176, 179, 182, 185, 187, 190, 193, 195, 198, 201, 203, 206, 208, 210, 213, 215,
  217, 219, 222, 224, 226, 228, 230, 231, 233, 235, 236, 238, 240, 241, 242, 244,
  245, 246, 247, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 254, 254, 254,
  255, 254, 254, 254, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 246,
  245, 244, 242, 241, 240, 238, 236, 235, 233, 231, 230, 228, 226, 224, 222, 219,
  217, 215, 213, 210, 208, 206, 203, 201, 198, 195, 193, 190, 187, 185, 182, 179,
  176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
  128, 124, 121, 118, 115, 112, 109, 106, 103, 100,  97,  94,  91,  88,  85,  82,
   79,  76,  73,  70,  68,  65,  62,  60,  57,  54,  52,  49,  47,  45,  42,  40,
   38,  36,  33,  31,  29,  27,  25,  24,  22,  20,  19,  17,  15,  14,  13,  11,
   10,   9,   8,   7,   6,   5,   4,   4,   3,   2,   2,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   2,   2,   3,   4,   4,   5,   6,   7,   8,   9,
   10,  11,  13,  14,  15,  17,  19,  20,  22,  24,  25,  27,  29,  31,  33,  36,
   38,  40,  42,  45,  47,  49,  52,  54,  57,  60,  62,  65,  68,  70,  73,  76,
   79,  82,  85,  88,  91,  94,  97, 100, 103, 106, 109, 112, 115, 118, 121, 124
};

void playSineWave(int frequency, int durationMs) {
  int sampleRate = frequency * 256;          // 256 samples per cycle
  int totalSamples = (sampleRate * durationMs) / 1000;
  int delayUs = 1000000 / sampleRate;        // Interval between samples (microseconds)
  
  for (int i = 0; i < totalSamples; i++) {
    dac_output_voltage(DAC_CHANNEL_2, sineWave[i % 256]);
    delayMicroseconds(delayUs);
  }
  
  dac_output_voltage(DAC_CHANNEL_2, 128);    // Return to mid-level (silence)
}

void loop() {
  playSineWave(1000, 1000);  // Play 1kHz sine wave for 1 second
  delay(2000);
}