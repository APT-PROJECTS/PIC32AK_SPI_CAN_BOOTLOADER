# Add project-owned sources that are not emitted by the MPLAB/MCC generated
# CMake source list.
#
# This file is included by the generated CMakeLists.txt after .generated/main.cmake,
# so it survives regeneration of the generated files.

set(_extra_project_sources
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/driver/device_stat.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/driver/driver_CANbus.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/driver/driver_mcp2515.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/driver/mcu_config.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/software/state_engine.c"
)

foreach(_source IN LISTS _extra_project_sources)
    set_source_files_properties("${_source}" PROPERTIES LANGUAGE C)
endforeach()

get_target_property(
    _generated_compile_sources
    PIC32AK64_SPI_CAN_BOOTLOADER_default_default_XC32_pic32a_compile
    SOURCES
)

set(_user_object_lib PIC32AK64_SPI_CAN_BOOTLOADER_user_sources)

if(NOT TARGET ${_user_object_lib})
    add_library(${_user_object_lib} OBJECT ${_extra_project_sources})
    PIC32AK64_SPI_CAN_BOOTLOADER_default_default_XC32_pic32a_compile_rule(${_user_object_lib})
endif()

get_property(_generated_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)

foreach(_target IN LISTS _generated_targets)
    if(_target MATCHES "^PIC32AK64_SPI_CAN_BOOTLOADER_default_image_")
        target_sources(${_target} PRIVATE $<TARGET_OBJECTS:${_user_object_lib}>)
    endif()
endforeach()
