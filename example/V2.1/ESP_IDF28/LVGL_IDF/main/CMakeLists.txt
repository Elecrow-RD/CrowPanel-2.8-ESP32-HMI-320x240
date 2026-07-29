idf_component_register(
    SRCS "CrowPanel_ESP32_2.8.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino-esp32 TFT_eSPI lvgl-3 UI DHT20
)
