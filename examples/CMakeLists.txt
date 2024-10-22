##############################################################################

set(NAME "adc")

add_executable(${NAME}
    common/main.c
    adc/run.c
)
target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../include
)
target_link_libraries(${NAME} picofuse)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${NAME})

##############################################################################

set(NAME "blink")

add_executable(${NAME}
    common/main.c
    blink/run.c
)
target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../include
)
target_link_libraries(${NAME} picofuse)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${NAME})

##############################################################################

set(NAME "bme280_example")

add_executable(${NAME}
    common/main.c
    bme280/run.c
)
target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../include
)
target_link_libraries(${NAME} 
    picofuse
    bme280
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${NAME})

##############################################################################

set(NAME "fade")

add_executable(${NAME}
    common/main.c
    fade/run.c
)
target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../include
)
target_link_libraries(${NAME} 
    picofuse
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${NAME})

##############################################################################

set(NAME "gpio_in")

add_executable(${NAME}
    common/main.c
    gpio_in/run.c
)
target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../include
)
target_link_libraries(${NAME} picofuse)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${NAME})

##############################################################################

set(NAME "inky")

add_executable(${NAME}
    common/main.c
    inky/run.c
)
target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../include
)
target_link_libraries(${NAME} 
    picofuse
    uc8151
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${NAME})
