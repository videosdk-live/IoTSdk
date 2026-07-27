idf_component_register(
    INCLUDE_DIRS "include"
    REQUIRES
        # Components the prebuilt library links against.
        mbedtls
        json
        esp_netif
        fatfs
        vfs
        esp_common
        esp_hw_support
        esp_timer
        esp_lcd
        esp-tls
        nvs_flash
        # ESP-IDF drivers it uses
        driver
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_i2s
        esp_driver_spi
        sdmmc
        # Registry components it uses
        espressif__mdns
        espressif__esp_audio_codec
        espressif__esp_codec_dev
        espressif__esp_audio_effects
        espressif__esp_video_codec
        espressif__esp_jpeg
        espressif__esp_new_jpeg
        espressif__esp32-camera
        espressif__esp_websocket_client
        sepfy__srtp
        sepfy__usrsctp
)

if(CONFIG_ESP32_S3_KORVO_2_V3_0_BOARD)
    set(prebuilt_lib_path "${CMAKE_CURRENT_SOURCE_DIR}/libiot-sdk-korvov2.a")
    set(prebuilt_lib_name "iot_sdk_korvov2_prebuilt")
    message(STATUS "Linking IoT SDK for ESP32-S3-Korvo-2")
elseif(CONFIG_ESP32S3_XIAO)
    set(prebuilt_lib_path "${CMAKE_CURRENT_SOURCE_DIR}/libiot-sdk-xiao.a")
    set(prebuilt_lib_name "iot_sdk_xiao_prebuilt")
    message(STATUS "Linking IoT SDK for ESP32-S3-XIAO")
else()
    set(prebuilt_lib_path "${CMAKE_CURRENT_SOURCE_DIR}/libiot-sdk-xiao.a")
    set(prebuilt_lib_name "iot_sdk_xiao_prebuilt")
    message(WARNING "Board not detected, defaulting to ESP32-S3-XIAO configuration")
endif()

# Link the prebuilt library. Repeating the same REQUIRES here is what makes it
# link cleanly (no undefined-reference errors), so keep this list in sync with
# the one above.
add_prebuilt_library(${prebuilt_lib_name} "${prebuilt_lib_path}"
    REQUIRES
        mbedtls
        json
        esp_netif
        fatfs
        vfs
        esp_common
        esp_hw_support
        esp_timer
        esp_lcd
        esp-tls
        nvs_flash
        driver
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_i2s
        esp_driver_spi
        sdmmc
        espressif__mdns
        espressif__esp_audio_codec
        espressif__esp_codec_dev
        espressif__esp_audio_effects
        espressif__esp_video_codec
        espressif__esp_jpeg
        espressif__esp_new_jpeg
        espressif__esp32-camera
        espressif__esp_websocket_client
        sepfy__srtp
        sepfy__usrsctp
)

target_link_libraries(${COMPONENT_LIB} INTERFACE ${prebuilt_lib_name})
