idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES mbedtls json esp_netif fatfs vfs esp_common esp_timer esp_lcd nvs_flash
)

# Fix for libsrtp pointer type mismatch on ESP32/Xtensa for esp-idf 5.4.
# On ESP32, uint32_t is 'long unsigned int' but srtp.c uses 'unsigned int'.
# Both are 4 bytes but the compiler treats them as incompatible pointer types.
idf_component_get_property(srtp_lib sepfy__srtp COMPONENT_LIB)
target_compile_options(${srtp_lib} PRIVATE -Wno-incompatible-pointer-types)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
