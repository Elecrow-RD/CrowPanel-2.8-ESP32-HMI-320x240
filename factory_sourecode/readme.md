### NOTE!!!:When running this program arduino factory program, you need to download the officially provided libraries and place them in the libraries of arduino. esp32 version needs to be 2.0.14/2.0.15. the version of the libraries is the same as the officially provided version.



## Key Functions in the Arduino Program

### `setup()`

The `**setup()**` function is called once when the program starts. It initializes the serial communication, sets up the display, touchpad, and initializes the LVGL graphics library. It also sets the display rotation and initializes the display driver and input device driver for LVGL.

- `Serial.begin(115200)`: Initializes the serial communication with a baud rate of 115200.
- `lcd.begin()`: Initializes the TFT display.
- `lv_init()`: Initializes the LVGL graphics library.
- `lv_disp_drv_register(&disp_drv)`: Registers the display driver with LVGL.
- `lv_indev_drv_register(&indev_drv)`: Registers the input device driver (touchpad) with LVGL.
- `ui_init()`: Initializes the user interface.

### `loop()`

The `**loop()**` function is called repeatedly after `setup()`. It handles the main program logic, updating the display with temperature and humidity data and toggling an LED based on the `led` variable.

- `lv_label_set_text(ui_Label1, DHT_buffer)`: Updates the text of a label on the UI with the temperature value.
- `lv_label_set_text(ui_Label2, DHT_buffer)`: Updates the text of a label on the UI with the humidity value.
- `digitalWrite(25, HIGH)` and `digitalWrite(25, LOW)`: Sets the state of the LED connected to pin 25.
- `lv_timer_handler()`: Calls the LVGL task handler to process GUI tasks.

### `my_disp_flush()`

The `**my_disp_flush()**` function is a callback function used by LVGL to flush the display buffer to the screen.

- `lcd.startWrite()`: Begins a write transaction to the TFT display.
- `lcd.setAddrWindow(area->x1, area->y1, w, h)`: Sets the address window for the TFT display.
- `lcd.pushColors((uint16_t *)&color_p->full, w * h, true)`: Sends the color data to the TFT display.
- `lv_disp_flush_ready(disp)`: Informs LVGL that the flushing is done.

### `my_touchpad_read()`

The `**my_touchpad_read()**` function is a callback function used by LVGL to read touchpad data.

- `lcd.getTouch(&touchX, &touchY, 600)`: Reads the touch coordinates from the TFT display.
- `data->state = LV_INDEV_STATE_PR` or `LV_INDEV_STATE_REL`: Sets the state of the touchpad (pressed or released).
- `data->point.x = touchX` and `data->point.y = touchY`: Sets the touch coordinates for LVGL.

### `clearBufferArray()`

The `**clearBufferArray()**` function clears the serial buffer array.

- `buffer[i] = NULL`: Sets each element of the buffer array to NULL, effectively clearing it.

