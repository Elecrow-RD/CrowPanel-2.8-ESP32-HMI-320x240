# CrowPanel ESP32 2.8-inch LVGL demo

ESP-IDF project for the Elecrow CrowPanel ESP32 2.8-inch HMI display (320x240,
ILI9341 SPI LCD and resistive touch). The UI displays DHT20 temperature and
humidity readings and provides two touch buttons that control GPIO 25.

## Requirements

- Elecrow CrowPanel ESP32 2.8-inch HMI display, hardware V2.1
- ESP-IDF 5.5.x (tested with 5.5.4)
- Internet access during the first configure so ESP-IDF Component Manager can
  download Arduino-ESP32 3.3.8 and LVGL 9.1.0

## Build and flash

Open an ESP-IDF terminal in this directory, then run:

```powershell
idf.py set-target esp32
idf.py build
idf.py -p COM41 flash monitor
```

Replace `COM41` with the serial port used by your board. Exit the monitor with
`Ctrl+]`.

The project is configured for a 4 MiB flash chip, DIO mode, 40 MHz flash clock,
and a 1000 Hz FreeRTOS tick. Hardware pin assignments and the ILI9341 setup are
in `components/TFT_eSPI/User_Setup.h`.

If no DHT20 is available, startup prints `DHT20 init failed` once and the UI
continues to run using its initial label values.

## Repository contents

- `main/`: board initialization and application loop
- `components/UI/`: SquareLine-generated LVGL screen and image assets
- `components/DHT20/`: minimal DHT20 driver
- `components/TFT_eSPI/`: display/touch driver and board configuration
- `sdkconfig.defaults`: reproducible project configuration
- `dependencies.lock`: locked Component Manager dependency versions

LVGL, Arduino-ESP32, TFT_eSPI, and the generated UI assets retain their
respective upstream licenses and notices.
