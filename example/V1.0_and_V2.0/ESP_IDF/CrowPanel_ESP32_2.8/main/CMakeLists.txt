file(GLOB_RECURSE LIB_SRCS
        "..//components/DHT20/*.CPP"
)

idf_component_register(SRCS "CrowPanel_ESP32_2.8.cpp"
                    ${LIB_SRCS}
                    INCLUDE_DIRS "."
                    "../components/arduino-esp32-master"
                    "../components/TFT_eSPI"
                    "../components/lvgl-3"
                    "../components/UI"
                    "../components/DHT20"
                    )
