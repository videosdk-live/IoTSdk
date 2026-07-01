idf_component_register(
    INCLUDE_DIRS "include"
    REQUIRES
        # Mirrors REQUIRES from the source component
        mbedtls
        json
        esp_netif
        fatfs
        vfs
        esp_common
        esp_timer
        esp_lcd
        esp-tls
        nvs_flash
        # IDF drivers the prebuilt .a calls into directly
        driver
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_i2s
        sdmmc
        # Registry components used by the prebuilt .a
        espressif__mdns
        espressif__esp_audio_codec
        espressif__esp_codec_dev
        espressif__esp_audio_effects
        espressif__esp_capture
        espressif__esp_video_codec
        espressif__esp_jpeg
        espressif__esp32_camera
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

# Register the prebuilt static library as an IMPORTED target with explicit
# transitive REQUIRES so CMake places its dependency archives AFTER the
# prebuilt on the linker command line. Without this, `ld` (single-pass over
# archives) sees undefined references to esp_capture_*, esp_audio_*,
# esp_codec_dev_*, etc. when this component is loaded after its providers
# (which is namespace-order dependent).
add_prebuilt_library(${prebuilt_lib_name} "${prebuilt_lib_path}"
    REQUIRES
        mbedtls
        json
        esp_netif
        fatfs
        vfs
        esp_common
        esp_timer
        esp_lcd
        esp-tls
        nvs_flash
        driver
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_i2s
        sdmmc
        espressif__mdns
        espressif__esp_audio_codec
        espressif__esp_codec_dev
        espressif__esp_audio_effects
        espressif__esp_capture
        espressif__esp_video_codec
        espressif__esp_jpeg
        espressif__esp32_camera
        espressif__esp_websocket_client
        sepfy__srtp
        sepfy__usrsctp
)

target_link_libraries(${COMPONENT_LIB} INTERFACE ${prebuilt_lib_name})
