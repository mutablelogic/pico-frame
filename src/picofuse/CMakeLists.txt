set(NAME "picofuse")
add_library(${NAME} STATIC
    adc.c
    gpio.c
    led.c
    pwm.c
    spi.c
    watchdog.c
)

target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../include
    ${CMAKE_CURRENT_LIST_DIR}/../fuse
)

target_link_libraries(${NAME} 
    fuse
)

if(${TARGET_OS} STREQUAL "pico")
    target_link_libraries(${NAME}
        hardware_adc
        hardware_pwm        
        hardware_spi
        hardware_watchdog
        pico_stdlib
    )
    if(PICO_CYW43_SUPPORTED)
        target_link_libraries(${NAME}
            pico_cyw43_arch_lwip_threadsafe_background
            pico_lwip_mqtt
        )
        target_compile_definitions(${NAME} PRIVATE PICO_CYW43_SUPPORTED=1)
    endif()
endif()

add_subdirectory(drivers/bme280)
add_subdirectory(drivers/uc8151)
