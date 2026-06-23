# Add project-owned sources that are not emitted by the MPLAB/MCC generated
# CMake source list.
#
# This file is included by the generated CMakeLists.txt after .generated/main.cmake,
# so it survives regeneration of the generated files.

set(_boot_handler_source "${CMAKE_CURRENT_LIST_DIR}/../../../src/middleware/boot_handler.c")
set_source_files_properties("${_boot_handler_source}" PROPERTIES LANGUAGE C)

get_target_property(
    _generated_compile_sources
    PIC32AK64_SPI_CAN_BOOTLOADER_default_default_XC32_pic32a_compile
    SOURCES
)

list(FIND _generated_compile_sources "${_boot_handler_source}" _boot_handler_index)

if(_boot_handler_index EQUAL -1)
    add_library(PIC32AK64_SPI_CAN_BOOTLOADER_user_boot_handler OBJECT "${_boot_handler_source}")
    PIC32AK64_SPI_CAN_BOOTLOADER_default_default_XC32_pic32a_compile_rule(PIC32AK64_SPI_CAN_BOOTLOADER_user_boot_handler)

    get_property(_generated_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)

    foreach(_target IN LISTS _generated_targets)
        if(_target MATCHES "^PIC32AK64_SPI_CAN_BOOTLOADER_default_image_")
            target_sources(${_target} PRIVATE $<TARGET_OBJECTS:PIC32AK64_SPI_CAN_BOOTLOADER_user_boot_handler>)
        endif()
    endforeach()
endif()
