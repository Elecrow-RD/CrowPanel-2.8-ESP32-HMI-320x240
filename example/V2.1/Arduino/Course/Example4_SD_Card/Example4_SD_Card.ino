#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

/*---------------------------------------------------------------
 * microSD hardware configuration
 * These pins connect the ESP32 SPI bus to the on-board card socket.
 *--------------------------------------------------------------*/
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18
#define SD_CS 5

/**
 * @brief Initialize the serial monitor, SPI bus, and microSD card.
 *
 * The Arduino runtime calls this function once after startup or reset. The
 * return value from SD_init() determines which status message is printed.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(9600);
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  delay(100);

  if (SD_init() == 1) {
    Serial.println("Card Mount Failed");
  } else {
    Serial.println("initialize SD Card successfully");
  }
}

/**
 * @brief Keep the card test idle after its one-time directory scan.
 *
 * The Arduino runtime calls this function repeatedly after setup().
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}

/**
 * @brief Mount the microSD card, report its size, and list its files.
 *
 * setup() calls this function once after the SPI bus is ready. A failure is
 * returned when the card cannot be mounted or no card type is detected.
 *
 * @param None.
 * @return 0 when initialization succeeds.
 * @return 1 when the card cannot be used.
 */
int SD_init() {
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return 1;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No TF card attached");
    return 1;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("TF Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 2);

  // The following operations are optional exercises and remain disabled.
  //  listDir(SD, "/", 0);
  //  createDir(SD, "/mydir");
  //  listDir(SD, "/", 0);
  //  removeDir(SD, "/mydir");
  //  listDir(SD, "/", 2);
  //  writeFile(SD, "/hello.txt", "Hello ");
  //  appendFile(SD, "/hello.txt", "World!\n");
  //  readFile(SD, "/hello.txt");
  //  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  //  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  //  Serial.println("SD init over.");

  return 0;
}

/**
 * @brief Recursively print files from a directory on the card.
 *
 * SD_init() first calls this function for the root directory. Each recursive
 * call reduces levels so traversal cannot continue deeper than requested.
 *
 * @param fs Mounted file-system object.
 * @param dirname Directory path to open.
 * @param levels Maximum number of nested directory levels to visit.
 * @return Nothing.
 */
void listDir(fs::FS & fs, const char *dirname, uint8_t levels) {
  //  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    // Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  // i = 0;
  while (file) {
    if (file.isDirectory()) {
      // Serial.print("  DIR : ");
      // Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("FILE: ");
      Serial.print(file.name());
      // lcd.setCursor(0, 2 * i);
      // lcd.printf("FILE:%s", file.name());

      Serial.print("SIZE: ");
      Serial.println(file.size());
      // lcd.setCursor(180, 2 * i);
      // lcd.printf("SIZE:%d", file.size());
      // i += 16;
    }

    file = root.openNextFile();
  }
}
