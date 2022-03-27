cmake_minimum_required(VERSION 3.20) # 3.21+ does not work!

# Add -O0 to remove optimizations when using gcc
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

OPTION(OPTION_ENVIRONMENT_LOCAL "Build application for local development" OFF)

if(OPTION_ENVIRONMENT_LOCAL)
else()
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
    include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
endif()

project(ws2812 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(OPTION_ENVIRONMENT_LOCAL)
else()
    pico_sdk_init()
endif()


set (SRC_LIST
    global.h
    storage.h
    storage.c
    led_math.h
    led_math.c
    options.h
    options.c
    easing.h
    easing.c
    colors.h
    colors.c
    perlinnoise.h
    perlinnoise.c
    actions.h
    actions.c
    patterns.h
    
    # Figure out how to include these dynamically
    patterns/pattern_bouncer.c
    patterns/pattern_fade_between.c
    patterns/pattern_fill_sway.c
    patterns/pattern_rainbow_wave.c
    patterns/pattern_random.c
    patterns/pattern_snakes.c
    patterns/pattern_sparkle.c
    patterns/pattern_strobe.c
    
    patterns.c

    ws2812.c
)

if(OPTION_ENVIRONMENT_LOCAL)
   set (SRC_LIST environment_local.h environment_local.c ${SRC_LIST})
else()
    set (SRC_LIST environment_pico.h environment_pico.c ${SRC_LIST})
endif()

add_executable(${PROJECT_NAME}
    environment.h
    environment.c
    ${SRC_LIST}
)

if(OPTION_ENVIRONMENT_LOCAL)
else()
    pico_enable_stdio_usb(${PROJECT_NAME} 0)
    pico_enable_stdio_uart(${PROJECT_NAME} 1)
    pico_generate_pio_header(${PROJECT_NAME}
        ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
    )

    target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_pio
        hardware_adc
        hardware_pwm
        hardware_flash
        hardware_sync
        pico_multicore
    )

    pico_add_extra_outputs(${PROJECT_NAME})
endif()