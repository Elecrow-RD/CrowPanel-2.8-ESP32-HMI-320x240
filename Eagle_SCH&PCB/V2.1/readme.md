# CrowPanel ESP32 Display 2.8 V2.1 Product Hardware Driver Documentation

| Item | Content |
|---|---|
| Document Version | V1.0 |
| Document Date | 2026-07-29 |
| Applicable Hardware | CrowPanel ESP32 Display 2.8, PCB/Schematic V2.1 (2024-03-14) |
| Main Controller | ESP32-WROOM-32-N4 |
| Software Baseline | The `Arduino/Course` verified examples in this repository and the accompanying driver libraries |
| Author | Codex (compiled from project materials) |
| Verification Principle | Running code takes priority; schematics are used to confirm electrical connections; content not covered by code is explicitly marked as "Schematic Confirmed / Pending Physical Verification" |

## 1. Document Purpose and Evidence Scope

This document is intended for hardware maintenance, Arduino driver porting, and onboarding of new team members. Cross-verification follows the evidence priority below:

1. The actual configuration of the project examples in `Arduino/Course` and `TFT_eSPI/User_Setup.h`;
2. The net names and component connections in `CrowPanel ESP32 Display-2.8-V2.1-20240314.sch/.pdf/.brd`;
3. The protocols, timing, and default parameters in the source code of the accompanying libraries;
4. The general behavior of the ESP32 Arduino Core/ESP-IDF.

"Verified" only means that the repository delivers it as working code, and does not equate to re-running electrical tests on physical hardware this time. The schematic contains a few imprecise namings and annotations; the discrepancies are detailed in Section 5.

## 2. Peripheral Overview

| Category | Device/Function | Onboard/External | MCU Pin | Interface/Driver | Evidence Status |
|---|---|---|---|---|---|
| Main Controller | ESP32-WROOM-32-N4 (4 MB Flash) | Onboard | Global | Arduino Core + ESP-IDF | Schematic and code consistent |
| Display | 2.8-inch 320x240 TFT, ILI9341 | Onboard module | 12/13/14/15/2/27 | SPI + GPIO | TFT/LVGL examples available |
| Touch | XPT2046 resistive touch | Onboard | 12/13/14/33, IRQ 36 | Shared SPI, independent CS | Touch/LVGL examples available |
| Storage | microSD/TF card slot | Onboard | 18/19/23/5 | VSPI/SD SPI | Card-read example available |
| Audio | SC8002B amplifier and speaker interface | Onboard | 26 | DAC2 analog output | Sine-wave example available |
| Indicator | Red user LED | Onboard | 25 | GPIO push-pull, active high | Blink/UI examples available |
| Buttons | BOOT, RESET | Onboard | 0, EN | Active-low buttons | Schematic confirmed |
| USB | USB Type-C + CH340C | Onboard | UART0 GPIO1/3; EN/IO0 auto-download | USB 2.0 FS to UART | Schematic confirmed, serial example covered |
| Wireless | 2.4 GHz Wi-Fi | MCU built-in | RF internal | ESP32 Wi-Fi stack | STA example available |
| Wireless | Bluetooth Low Energy | MCU built-in | RF internal | ESP32 BLE stack | GATT Server example available |
| Power | RY3420 3.3 V Buck | Onboard | No software control | VIN -> 3.3 V | Schematic confirmed |
| Battery | 4054A single-cell Li-ion charging, PMOS/Schottky power path | Onboard | No software control | VBUS/BAT+ -> VIN | Schematic confirmed |
| Expansion | I2C 4-pin interface | Onboard interface | SDA 22, SCL 21 | I2C | DHT20/OLED examples covered |
| Sensor | DHT20 temperature/humidity module, address 0x38 | External I2C | SDA 22, SCL 21 | Hardware I2C | LVGL comprehensive example covered |
| Display Expansion | SSD1306 128x64 OLED | External I2C | SDA 22, SCL 21 | U8g2 software I2C | Example available |
| Expansion | UART2 4-pin interface / GPS module | External | RX 16, TX 17 | UART 9600 8N1 | GPS passthrough example covered |
| Expansion | GPIO/ADC 4-pin interface | Onboard interface | 25, 32 (plus 4/34/35/39 net breakout capability) | GPIO/ADC/DAC/Touch | Schematic confirmed; 25 verified |

## 3. Overall Pin Assignment and Resource Conflict Matrix

| GPIO | Current Function | Direction/Multiplex | Power-on or Electrical Notes |
|---:|---|---|---|
| 0 | BOOT | Input, active low | Strapping pin; pulled low during download; ordinary peripherals must not forcibly pull it low during reset |
| 1 | UART0 TX | Output | Connected to CH340C via 22 Ω series resistor; used for download/logging |
| 2 | TFT D/C (RS) | Push-pull output | Strapping-related pin; external circuitry must not disturb the power-on level |
| 3 | UART0 RX | Input | Connected to CH340C via 22 Ω series resistor |
| 4 | Expansion GPIO/ADC/Touch | Bidirectional | ADC2 cannot sample reliably while Wi-Fi is active |
| 5 | microSD CS | Push-pull output, idle high | Strapping-related pin; 10 kΩ pull-up on board |
| 12 | TFT/Touch MISO | SPI input | ESP32 MTDI strapping pin; schematic has optional resistor configuration, peripherals must not drive strongly at power-on |
| 13 | TFT/Touch MOSI | SPI output | Shared by TFT and touch |
| 14 | TFT/Touch SCLK | SPI output | Shared by TFT and touch |
| 15 | TFT CS | Push-pull output, idle high | Strapping-related pin |
| 16 | UART2 RX | Input | Connects to peripheral TX; 3.3 V TTL |
| 17 | UART2 TX | Output | Connects to peripheral RX; 3.3 V TTL |
| 18 | microSD SCLK | VSPI output | SD dedicated bus |
| 19 | microSD MISO | VSPI input | SD dedicated bus |
| 21 | I2C SCL | Open-drain bidirectional | Schematic net name `IO21_SCL`, interface has pull-up |
| 22 | I2C SDA | Open-drain bidirectional | Schematic net name `IO22_SDA`, interface has pull-up |
| 23 | microSD MOSI | VSPI output | SD dedicated bus |
| 25 | User LED/expansion port | Push-pull output or DAC1/ADC2 | LED active high; shared with expansion port, cannot be used for external analog signal at the same time |
| 26 | Audio amplifier input | DAC2 output | Code uses 8-bit DAC; should not be used as a digital bus pin |
| 27 | LCD backlight control | Push-pull/PWM output | Active high, controls the backlight loop via 2N7002 |
| 32 | Expansion GPIO/ADC1/Touch | Bidirectional | Shared with J7 expansion interface |
| 33 | XPT2046 CS | Push-pull output, idle high | Touch dedicated chip select |
| 34/35/39 | Expansion ADC1 | Input only | No internal pull-up/pull-down; cannot be configured as output |
| 36 | XPT2046 PENIRQ | Input only, interrupt optional | Low level indicates touch; current TFT_eSPI example uses polling and does not use the IRQ |

The display/touch uses HSPI-style pins 12/13/14 and the SD uses VSPI pins 19/23/18; the two buses can operate independently. Do not repeatedly reassign the pins of the Arduino global `SPI` at runtime; when combining SD and TFT, you should explicitly use the corresponding `SPIClass` or ensure each library uses a different SPI controller.

## 4. Per-Peripheral Driver Description

### 4.1 ESP32-WROOM-32-N4 Main Controller

- Power: 3.3 V; the schematic uses RY3420 to generate the system 3.3 V.
- Flash: The module model suffix N4 is configured as 4 MB of external SPI Flash; GPIO6~11 are reserved for the module Flash and must not be used externally.
- Software layer: Arduino Core for ESP32; the underlying layer uses the ESP-IDF GPIO, SPI, I2C, UART, DAC, Wi-Fi, and BLE drivers.
- Boot: IO0 at low level on reset enters download mode; normal high level boots from SPI Flash. CH340C's DTR/RTS connect to IO0/EN via transistors to enable auto-download.
- Recommendation: The board type should select a target compatible with ESP32 Dev Module/WROOM-32; avoid selecting ESP32-S2/S3/C3 because the GPIO, DAC, and wireless APIs are all different.

### 4.2 ILI9341 TFT LCD and Backlight

| Signal | GPIO | Connection/Mode |
|---|---:|---|
| SDO/MISO | 12 | SPI input, shared by LCD/touch |
| SDI/MOSI | 13 | SPI output, shared by LCD/touch |
| SCL/SCLK | 14 | SPI clock, shared by LCD/touch |
| CS | 15 | Active-low chip select |
| D/C (RS) | 2 | Command/data select |
| RESET | -1 | `TFT_RESET` connected to system `EN_RESET` via 0 Ω, software does not occupy a GPIO |
| BL | 27 | Active high; GPIO drives the 2N7002 backlight control stage |

Valid configuration in the accompanying `TFT_eSPI/User_Setup.h`: ILI9341, 240x320, write clock 15,999,999 Hz, read clock 20 MHz, touch clock 600 kHz. The code uses `setRotation(1)` in landscape mode, logical resolution 320x240.

```cpp
// TFT_eSPI/User_Setup.h
#define ILI9341_DRIVER
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   27
#define SPI_FREQUENCY       15999999
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY 600000
```

Initialization sequence: first `lcd.begin()`, clear the screen, wait about 300 ms, then pull GPIO27 high, and finally set the rotation direction. If dimming is required, change GPIO27 to an ESP32 LEDC PWM; keep it active high, and do not drive the backlight LED current directly.

LVGL uses 320x240 with a 1/8-frame partial buffer; the refresh callback sends pixels via `startWrite()`/`setAddrWindow()`/`pushColors()`/`endWrite()` and calls `lv_display_flush_ready()` after completion.

### 4.3 XPT2046 Resistive Touch

The touch and LCD share GPIO12/13/14, with an exclusive CS=GPIO33; the schematic also connects PENIRQ to GPIO36. The current working code polls via TFT_eSPI and does not configure the GPIO36 interrupt.

- SPI: Mode 0; TFT_eSPI configured at 600 kHz.
- Chip select: active low, should remain high when idle.
- Sampling: `lcd.getTouch(&x, &y, 600)`, pressure threshold 600.
- Calibration: landscape `rotation=1` uses `{557, 3263, 369, 3493, 3}`; the four-corner calibration must be re-run after a panel replacement.
- LVGL coordinates: the comprehensive example uses `x = 320 - touchX`, `y = touchY` for landscape mirroring.

```cpp
uint16_t calData[5] = {557, 3263, 369, 3493, 3};
lcd.begin();
lcd.setRotation(1);
lcd.setTouch(calData);
bool pressed = lcd.getTouch(&touchX, &touchY, 600);
```

The accompanying standalone `XPT2046_Touchscreen` library defaults to 2 MHz, SPI Mode 0, MSB first, which differs from TFT_eSPI's 600 kHz. The current product baseline should follow the TFT_eSPI configuration; if switching to the standalone library, signal integrity and touch stability must be re-verified on the complete unit.

### 4.4 microSD/TF Card

| Signal | GPIO | SD SPI Meaning |
|---|---:|---|
| SCK | 18 | Clock |
| MISO | 19 | DATA0, card to MCU |
| MOSI | 23 | CMD, MCU to card |
| CS | 5 | CD/DATA3, active low |

```cpp
SPI.begin(18, 19, 23);       // SCK, MISO, MOSI
delay(100);
if (!SD.begin(5)) { /* mount failed */ }
```

The software layer is Arduino `SPI` + `SD` + `FS`. The example does not explicitly pass a frequency to `SD.begin()`, so it uses the current ESP32 SD library's default initialization/negotiated clock; when porting, do not present an unspecified frequency as a fixed commitment. The schematic configures 10 kΩ pull-ups on the CS, CMD, DATA0, and other lines, suitable for the SPI power-on idle state. Unmount the file system before hot-plugging; a power loss during writing will corrupt the FAT.

### 4.5 Speaker Amplifier SC8002B

- GPIO26 is also ESP32 DAC channel 2; it connects to the SC8002B amplifier via an analog network.
- The amplifier is an onboard analog stage that outputs to the 2-pin speaker interface; software cannot treat GPIO26 directly as the speaker power output.
- The example enables `DAC_CHANNEL_2` and outputs a 256-point 8-bit sine table. A 1 kHz tone corresponds to a theoretical 256 ksample/s and is timed in software via `delayMicroseconds()`; the actual frequency has scheduling and integer-truncation errors.

```cpp
dac_output_enable(DAC_CHANNEL_2);
dac_output_voltage(DAC_CHANNEL_2, sample); // sample: 0..255
```

The software depends on the ESP-IDF `driver/dac.h` (used through an Arduino project). When porting to a newer ESP-IDF, the old DAC API may be deprecated and should be migrated to the DAC oneshot/continuous driver. Writing the midpoint 128 on power-up/stop reduces the DC step but cannot fully eliminate the pop; for higher-quality audio, use a timer/DMA/I2S external DAC.

### 4.6 User LED, BOOT, and RESET

- User LED: GPIO25, with a 1 kΩ current-limiting resistor, code `HIGH` turns it on and `LOW` turns it off; GPIO push-pull output.
- BOOT: GPIO0, pulled low when pressed; combined with RESET to enter ROM download mode.
- RESET: EN, pulled low when pressed; CH340C DTR/RTS can also control it automatically.
- Power LED: directly connected to the 3.3 V power rail, no software control.

```cpp
pinMode(25, OUTPUT);
digitalWrite(25, HIGH); // LED on
```

GPIO25 also appears on the GPIO/ADC expansion interface; external devices may interact with the LED load; before performing DAC1 or ADC2 measurements, evaluate the load caused by the LED and its current-limiting resistor.

### 4.7 I2C Expansion, DHT20, and SSD1306 OLED

The J6 four-pin interface includes 3.3 V, GND, SCL=GPIO21, SDA=GPIO22. I2C is an open-drain bus with 1 kΩ series resistors on board; external modules often have their own pull-ups, so the equivalent pull-up after parallel combination should be calculated and should not be too small.

#### DHT20

- Address: 7-bit `0x38`.
- Software: Arduino `Wire` + `Crowbits_DHT20`.
- Initialization: `Wire.begin(22, 21)` follows the ESP32 parameter order of `(SDA, SCL)`; the sensor `begin()` first waits 100 ms, then sends 0x71 to read the status.
- Measurement: send `{0xAC, 0x33, 0x00}`, check the busy bit every 10 ms, up to 10 times, and read 6 bytes.
- The code does not explicitly set the I2C clock and uses the Arduino Core default value (typically 100 kHz, but this should be confirmed against the Core version used).

```cpp
Wire.begin(22, 21);       // SDA, SCL
Crowbits_DHT20 dht20;     // Wire, address 0x38
dht20.begin();
int t = dht20.getTemperature();
int rh = dht20.getHumidity();
```

The current comprehensive example triggers one temperature conversion and one humidity conversion in each ~10 ms UI loop; the frequency is clearly too high, and the two results are not guaranteed to come from the same sampling. The product firmware is recommended to read once every 1~2 s and to read once while parsing both temperature and humidity together.

#### SSD1306 OLED (External)

The example uses `U8G2_SSD1306_128X64_NONAME_F_SW_I2C`, meaning U8g2 implements software I2C on GPIO21/22; no address is explicitly given and the constructor's library default address is used. It is initialized as 128x64, no reset pin, direction R0, full buffer.

```cpp
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
    U8G2_R0, /* clock */ 21, /* data */ 22, U8X8_PIN_NONE);
u8g2.begin();
```

The OLED and DHT20 can share I2C electrically, but the current OLED software I2C instance does not arbitrate with `Wire`. When integrating, it is recommended to switch to the U8g2 hardware I2C constructor, use `Wire` uniformly, and confirm there is no address conflict.

### 4.8 UART2 Expansion and GPS

The J10 four-pin interface includes 3.3 V, GND, RXD2=GPIO16, TXD2=GPIO17. The level is 3.3 V TTL, not RS-232 level.

```cpp
HardwareSerial gpsSerial(2);
gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
```

The GPS example performs bidirectional passthrough of NMEA/commands at 9600 baud, 8 data bits, no parity, 1 stop bit; the USB debug serial port is also set to 9600 baud. The comprehensive LVGL example only calls `Serial2.begin(9600)` without explicitly mapping RX/TX, relying on the board's default mapping; portable code should always explicitly specify 16/17.

### 4.9 UART0, USB Type-C, and CH340C

- USB Type-C serves only as a USB 2.0 device/power input, with a 5.1 kΩ pull-down on each of CC1/CC2.
- D+/D- connect to CH340C via 22 Ω series resistors; the CH340C UART connects to ESP32 GPIO3/1 via 22 Ω.
- DTR/RTS automatically control EN/IO0 via S9013 transistors.
- Software uses Arduino `Serial`; the example baud rates include both 9600 and 115200, and the serial monitor must match the current firmware.
- The USB port is not the ESP32's native USB data port, and application code cannot use it as a USB-OTG/USB Device peripheral.

### 4.10 Wi-Fi and BLE

The wireless functions are integrated inside the ESP32 module and are not driven by external GPIO. The software depends on the ESP32 Arduino Core's Wi-Fi/BLE stack.

The Wi-Fi example uses 2.4 GHz STA mode: `WiFi.begin(ssid, password)`, with auto-reconnect, blocking until connected. The repository example contains plaintext demo credentials; the product firmware must switch to a secure configuration/provisioning mechanism and should add timeout and offline fallback.

The BLE example creates a GATT Server named `ESP32SPI-BLE`:

- Service UUID: `6479571c-2e6d-4b34-abe9-c35116712345`
- Characteristic UUID: `826f072d-f87c-4ae6-a416-6ffdcaa02d73`
- Properties: Read, Write, Notify; initial value `ELECROW`

The current code starts advertising first and then starts the service, and does not add the `BLE2902` descriptor nor restart advertising in the disconnect callback. For production, it is recommended to first call `pService->start()`, then configure and start advertising; if standard client subscription notifications are needed, add the CCCD and verify reconnection after disconnection.

### 4.11 Power Management and Battery

#### USB/Battery Input and Charging

- USB VBUS is the 5 V input.
- U26 is labeled 4054A, a single-cell Li-ion linear charger; the schematic notes a charging current of 500 mA, RPROG=2 kΩ.
- J5 is the battery interface, with net name `BAT+`; it is only for single-cell Li-ion/Li-Po cells.
- The PMOS 3401 and IN5817 form the USB/battery power path, outputting `VIN`.
- This circuit has no MCU-readable charge status, no precise fuel gauge, and no software shutdown interface; do not claim battery percentage support in documentation/APIs.

#### System 3.3 V

U1 is labeled RY3420, with feedback resistors 45.3 kΩ/10 kΩ; the schematic gives `Vout=0.6*(R20/R21+1)=3.3 V`. EN is connected to VIN, so it auto-enables on power-up with no GPIO control. The power and charging stages operate autonomously in hardware and require no driver initialization.

Maintenance note: USB 5 V, BAT+, VIN, and 3.3 V are different power domains; the expansion interface only allows 3.3 V logic. External modules must not feed 5 V into GPIO, nor draw from the 3.3 V interface more current than the regulator, connector, and thermal capacity can support.

### 4.12 GPIO/ADC Expansion Port

The schematic shows that J7 breaks out GPIO25, GPIO32, 3.3 V, and GND; additionally, the main controller nets GPIO4/34/35/39 are marked as expandable GPIO/ADC, but their specific accessible positions on this board should be confirmed against the PCB test points/pads.

- GPIO32/34/35/39 belong to ADC1 and can continue to use the ADC while Wi-Fi is active.
- GPIO4/25 belong to ADC2, and ADC2 is usually occupied by the wireless driver after Wi-Fi starts.
- GPIO34/35/36/39 are input only and have no internal pull-up/down.
- ESP32 ADC nonlinearity and chip-to-chip variation are significant; when measuring voltage, use attenuation settings and the calibration API, the input must never exceed 3.3 V, and the actual full scale also depends on the attenuation configuration.

## 5.
# Schematic vs. Code Discrepancy Record

| Item | Schematic / Library Info | Working Code | Adopted Conclusion and Possible Cause |
|---|---|---|---|
| I2C parameter order | Net names `IO21_SCL`, `IO22_SDA` | `Wire.begin(22, 21)` | Fully consistent; the ESP32 API order is SDA, SCL, and must not be mistakenly swapped to follow the textual order of appearance. |
| OLED I2C type | Board provides a generic I2C interface | Comment states software I2C, and the U8g2 constructor is indeed SW I2C | The OLED example uses software I2C; integration is recommended to switch to hardware `Wire`. |
| LCD RESET | The LCD interface has `TFT_RESET`, connected to `EN_RESET` via R5 (0 Ω); an optional R8/C28 network is also present | `TFT_RST=-1` | Follow the code: the LCD follows the system reset and is not assigned a GPIO. |
| LCD SPI write frequency | Not specified in the schematic | 15,999,999 Hz | Follow the bundled TFT_eSPI configuration; do not round it up to 40 MHz. |
| Touch SPI frequency | The standalone XPT2046 library is 2 MHz | TFT_eSPI configured at 600 kHz | The product baseline uses TFT_eSPI at 600 kHz; the two libraries have different default values. |
| Touch IRQ | GPIO36 connected to PENIRQ | The working code uses `getTouch()` polling | The IRQ is a reserved capability; do not claim that the current firmware already uses interrupts. |
| UART interface title | The schematic block is labeled "UART1 interface", with nets RXD2/TXD2 | The code uses `HardwareSerial(2)` | Follow the nets and code, and refer to it as UART2; the schematic block title may have carried over from an older revision. |
| `Serial2.begin(9600)` | The schematic fixes GPIO16/17 | The LVGL integrated example does not explicitly specify pins; the GPS example does | For porting, use the GPS example `begin(9600, SERIAL_8N1, 16, 17)`. |
| LED and expansion port | GPIO25 is connected to both the LED and J7 | Both examples drive the LED with GPIO25 | Follow the code, but an external load on GPIO25 will cause conflicts. |
| DHT20 / OLED / GPS | The schematic only has expansion interfaces, not the modules themselves | The course code provides module drivers | Classify them as "externally verified modules" and do not describe them as on-board devices. |
| DHT20 status check | The library logic `(data | 0x8) == 0` is almost never true | The integrated example does not check the `begin()` return value | Retain the current runnable conclusion, but this is a library-defect risk; fix it during porting and check the return code. |
| BLE startup order | Not related to the schematic | Broadcast first, then start the service | Usable for examples but not robust; product code should start the service before broadcasting. |

## 6. Recommended Initialization Sequence

1. Start `Serial` and output the firmware/board version and self-test log.
2. Set all chip selects high: TFT CS=15, Touch CS=33, SD CS=5, to prevent shared-bus devices from responding incorrectly.
3. Initialize I2C: `Wire.begin(22, 21)`; probe 0x38 and check the DHT20 return value.
4. Initialize the TFT, wait for the panel to stabilize, pull GPIO27 high or start the backlight PWM, set the rotation and touch calibration.
5. Initialize the LVGL display/input callbacks.
6. If SD is needed, initialize the SPI bus on GPIO18/19/23 and then mount the file system.
7. If GPS is needed, explicitly configure UART2 to GPIO16/17, 9600 8N1.
8. If audio is needed, enable DAC2 last, set the initial value to 128, to avoid startup pop.
9. Start high-power features such as Wi-Fi/BLE; verify the supply voltage drop and the impact on ADC2 resources.

## 7. Risks and Maintenance Notes

| Level | Risk | Recommendation |
|---|---|---|
| High | GPIO12, 2, 5, 15, 0 are involved in ESP32 strapping | They must not be forcibly pulled to an incorrect level by peripherals during reset; when a fault occurs, disconnect the expansion modules first for troubleshooting. |
| High | All MCU GPIOs are 3.3 V and generally not 5 V tolerant | UART/I2C/GPIO peripherals must use 3.3 V levels or reliable level shifting. |
| High | Single-cell Li-ion charging is nominally 500 mA, with heating and cell-matching requirements | Only connect a 1S cell with appropriate protection that allows this charging current; mass production requires temperature-rise testing. |
| High | GPIO25 is connected to both the LED and the expansion port | An external strong drive will cause bus contention; design an interface allocation table and avoid dual configuration. |
| Medium | LCD and touch share SPI, and incorrect CS management causes mutual interference | Use the correct SPI mode/frequency for each transaction, and keep the CS of unselected devices high. |
| Medium | The TFT configured frequency is the non-typical 15,999,999 Hz | Keep this verified value during porting; raising the frequency requires temperature/cable-length/batch testing. |
| Medium | The DHT20 example samples too fast and its driver error handling is insufficient | Reduce to a 1–2 s cycle, fetch temperature and humidity together in one conversion, and validate busy/CRC/read length. |
| Medium | GPIO34/35/36/39 are input-only with no internal pull-up/down | Floating inputs must have external biasing; output mode must not be called. |
| Medium | ADC2 conflicts with Wi-Fi resources | When wireless is running, prefer ADC1 on GPIO32/34/35/39. |
| Medium | Power loss during microSD writes | Add sync/close procedures and power-loss protection; avoid frequent small-block writes. |
| Medium | The GPS example uses a fixed 256-byte buffer with no protocol parsing | The product uses a ring buffer and TinyGPS++/NMEA parsing to handle continuous data streams. |
| Low | The examples contain plaintext Wi-Fi credentials | Remove them before release, use NVS/provisioning, and prevent credentials from entering the version control repository. |
| Low | The BLE example does not actively restart advertising after disconnection | Restart advertising in the disconnect callback or the main task. |

## 8. Porting Checklist

- [ ] The target chip is confirmed as a classic ESP32 (not S2/S3/C3), and the Arduino Core/ESP-IDF version is recorded.
- [ ] TFT_eSPI actually selects this repository's `User_Setup.h` at compile time, not the default configuration of the globally installed library.
- [ ] The ILI9341 orientation, color order, and 320x240 logical resolution are correct.
- [ ] Every touch panel is calibrated, and the pressure threshold and coordinate mirroring are measured.
- [ ] When SD and TFT are enabled simultaneously, both SPI controllers and the CS lifecycle are verified.
- [ ] UART2 is explicitly bound to GPIO16/17, and the peripheral level is 3.3 V TTL.
- [ ] The I2C pull-up equivalent resistance, address conflicts, and bus waveform are checked.
- [ ] After Wi-Fi is enabled, ADC2 is not used for critical measurements.
- [ ] Battery polarity, capacity, protection board, charging current, and overall temperature rise are confirmed.
- [ ] The strapping pins behave correctly in all three scenarios: cold start, reset, and auto-download.

## 9. Evidence Index

- Schematic: `V2.1/CrowPanel ESP32 Display-2.8-V2.1-20240314.sch`, `.pdf`
- PCB: `V2.1/CrowPanel ESP32 Display-2.8-V2.1-20240314.brd`
- TFT/LVGL: `Arduino/Course/LVGL_Arduino2.8/LVGL_Arduino2.8.ino`
- TFT/touch configuration: `Arduino/libraries/TFT_eSPI/User_Setup.h`
- Touch calibration: `Arduino/Course/Example5_Initialize_the_touch/Example5_Initialize_the_touch.ino`
- SD: `Arduino/Course/Example4_SD_Card/Example4_SD_Card.ino`
- Audio: `Arduino/Course/Example3_Speaker/Example3_Speaker.ino`
- LED: `Arduino/Course/Example1_LED_blinking/Example1_LED_blinking.ino`
- OLED: `Arduino/Course/Example2_OLED_screen/Example2_OLED_screen.ino`
- BLE/Wi-Fi: `Arduino/Course/Example6_BLE/Example6_BLE.ino`, `Example7_WIFI/Example7_WIFI.ino`
- GPS/UART2: `Arduino/Course/Example8_GPS_Module/Example8_GPS_Module.ino`
- DHT20: `Arduino/libraries/Crowbits_DHT20/Crowbits_DHT20.h/.cpp`

---

Maintenance convention: When the hardware is revised or the driver configuration changes, the "Peripheral Overview," "Pin Matrix," "Discrepancy Record," and document version must be updated synchronously; if measured conclusions conflict with this document, record the board batch, schematic version, firmware commit, and measurement evidence, then upgrade this document with the measured working configuration.
