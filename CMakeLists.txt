
idf_component_register(
    INCLUDE_DIRS "include" 
)

if(CONFIG_ESP32_S3_KORVO_2_V3_0_BOARD)
    target_link_libraries(${COMPONENT_LIB} INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/libiot-sdk-korvov2.a
    )
    message(STATUS "Linking IoT SDK for ESP32-S3-Korvo-2")
elseif(CONFIG_ESP32S3_XIAO)
    target_link_libraries(${COMPONENT_LIB} INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/libiot-sdk-xiao.a
    )
    message(STATUS "Linking IoT SDK for ESP32-S3-XIAO")
else()
    # Default to  if no specific board detected
    target_link_libraries(${COMPONENT_LIB} INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/libiot-sdk-xiao.a
    )
    message(WARNING "Board not detected, defaulting to ESP32-S3-XIAO configuration")
endif()