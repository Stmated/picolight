cmake_minimum_required(VERSION 3.13) # 3.21+ does not work!

#OPTION(OPTION_ENVIRONMENT_LOCAL "Build application for local development" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(OPTION_ENVIRONMENT_LOCAL)
    # Set some stuff if running locally?
else()
    
endif()

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picolight C CXX ASM)



#if(OPTION_ENVIRONMENT_LOCAL)
#    set(CMAKE_BUILD_TYPE Debug)
#endif()

#set(CMAKE_C_STANDARD 11)
#set(CMAKE_CXX_STANDARD 17)

set (SRC_LIST
    global.h
    types.h
    storage.h
    storage.c
    led_math.h
    led_math.c
    options.h
    options.c
    easing.h
    easing.c
    math/perlinnoise.h
    math/perlinnoise.c
    actions.h
    actions.c
    patterns.h
    math/simplerandom.h
    math/simplerandom.c
    
    # Figure out how to include these dynamically
    patterns/pattern_snake.c
    patterns/pattern_fade_between.c
    patterns/pattern_color_lerp.c
    patterns/pattern_hue_lerp.c
    patterns/pattern_rainbow_wave.c
    patterns/pattern_random.c
    patterns/pattern_random_sequence.c
    patterns/pattern_snakes.c
    patterns/pattern_sparkle.c
    patterns/pattern_gas_fade.c
    patterns/pattern_knightrider.c
    patterns/pattern_firework.c
    patterns/pattern_strobe.c
    patterns/pattern_meteor.c
    patterns/pattern_eyes.c
    patterns/pattern_test.c

    patterns.c

    picolight.c
)

if(OPTION_ENVIRONMENT_LOCAL)
   set (SRC_LIST environment/environment_local.h environment/environment_local.c ${SRC_LIST})
else()
    set (SRC_LIST environment/environment_pico.h environment/environment_pico.c ${SRC_LIST})
    # Initialise the Raspberry Pi Pico SDK
    pico_sdk_init()
endif()

add_executable(${PROJECT_NAME}
    environment/environment.h
    environment/environment.c
    ${SRC_LIST}
)

# pull in common dependencies and additional pwm hardware support
#target_link_libraries(pwm_led_fade pico_stdlib hardware_pwm)

# create map/bin/hex file etc.
#pico_add_extra_outputs(pwm_led_fade)

if(OPTION_ENVIRONMENT_LOCAL)
    #target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBS} )
else()
    # Make sure we use the fast float and double implementations
    pico_set_float_implementation(${PROJECT_NAME} pico)
    pico_set_double_implementation(${PROJECT_NAME} pico)

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