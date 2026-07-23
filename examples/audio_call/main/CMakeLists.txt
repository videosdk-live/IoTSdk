idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES mbedtls json esp_netif fatfs vfs esp_common esp_timer esp_lcd nvs_flash
)

# libsrtp needs this flag to build cleanly with the ESP32-S3 toolchain.
idf_component_get_property(srtp_lib sepfy__srtp COMPONENT_LIB)
target_compile_options(${srtp_lib} PRIVATE -Wno-incompatible-pointer-types)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
