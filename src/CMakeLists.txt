cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(sxgt-controller_project C CXX ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
pico_sdk_init()

if(APPLE)
execute_process(COMMAND xcrun -f swiftc OUTPUT_VARIABLE SWIFTC OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
execute_process(COMMAND which swiftc OUTPUT_VARIABLE SWIFTC OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

add_executable(sxgt-controller
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/swift_atomic_ops.c
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/_swiftcode.o
    COMMAND
        ${SWIFTC}
        -target armv6m-none-none-eabi -Xcc -mfloat-abi=soft -Xcc -fshort-enums
        -Xfrontend -function-sections -enable-experimental-feature Embedded -wmo -parse-as-library
        -Xcc -DCFG_TUSB_MCU=OPT_MCU_RP2040
        -Xllvm -mattr=+atomics-32
        -O
        $$\( echo '$<TARGET_PROPERTY:sxgt-controller,INCLUDE_DIRECTORIES>' | tr '\;' '\\n' | sed -e 's/\\\(.*\\\)/-Xcc -I\\1/g' \)
        $$\( echo '${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}'             | tr ' '  '\\n' | sed -e 's/\\\(.*\\\)/-Xcc -I\\1/g' \)
        -import-bridging-header ${CMAKE_CURRENT_LIST_DIR}/BridgingHeader.h
        ${CMAKE_CURRENT_LIST_DIR}/Main.swift
        -c -o ${CMAKE_CURRENT_BINARY_DIR}/_swiftcode.o
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/BridgingHeader.h
        ${CMAKE_CURRENT_LIST_DIR}/Main.swift
)

add_custom_target(sxgt-controller-swiftcode DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/_swiftcode.o)

target_include_directories(sxgt-controller PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(sxgt-controller
    pico_stdlib
    pico_unique_id
    hardware_gpio
    tinyusb_device
    tinyusb_board
    ${CMAKE_CURRENT_BINARY_DIR}/_swiftcode.o
)

add_dependencies(sxgt-controller sxgt-controller-swiftcode)
pico_add_extra_outputs(sxgt-controller)
