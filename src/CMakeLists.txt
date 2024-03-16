cmake_minimum_required(VERSION 3.12)
set(PICO_BOARD pico_w)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

project(Ziggy80 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_definitions(-DFAST_RDOP)

####
cmake_minimum_required(VERSION 3.15)

# Establecer la política CMP0115
cmake_policy(SET CMP0115 NEW)

add_executable(Ziggy80)
pico_generate_pio_header(Ziggy80 ${CMAKE_CURRENT_LIST_DIR}/z80pico.pio)

target_sources(Ziggy80 PRIVATE z80pico.c msx.c main.c marat/Z80.c ../pacman.c)

target_link_libraries(Ziggy80 PRIVATE
        pico_stdlib
        hardware_pio
        pico_cyw43_arch_none
        )

pico_add_extra_outputs(Ziggy80)

# add_executable(Test)
# pico_generate_pio_header(Test ${CMAKE_CURRENT_LIST_DIR}/z80pico.pio)

# target_sources(Test PRIVATE z80pico.c msx.c test.c marat/Z80.c)

# target_link_libraries(Test PRIVATE
#         pico_stdlib
#         hardware_pio
#         pico_cyw43_arch_none
#         )

# pico_add_extra_outputs(Test)
