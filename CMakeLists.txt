cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME comms-sw)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

include_directories(
        Lib/ECSS-Services/inc
        Lib/ECSS-Services/inc/etl
        Lib/ECSS-Services/inc/Helpers
        Lib/ECSS-Services/inc/Services
        Lib/ECSS-Services/inc/Time
)

file(GLOB_RECURSE ECSS_SOURCES "Lib/ECSS-Services/src/*.cpp" "Lib/ECSS-Services/src/Helpers/*.cpp" "Lib/ECSS-Services/src/Services/*.cpp" "Lib/ECSS-Services/src/Time/*.cpp")

# Create an executable object type
add_executable(
        ${CMAKE_PROJECT_NAME}
        ${ECSS_SOURCES}
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
# Add the eMMC Drivers
add_subdirectory(Drivers/eMMC)
# Add the AT Drivers
add_subdirectory(Drivers/AT86RF215)
# Add the INA3221 Drivers
add_subdirectory(Drivers/INA3221)
# Add the TMP117 Drivers
add_subdirectory(Drivers/TMP117)
# Add the GNSS drivers
add_subdirectory(Drivers/GNSS)
# Add the FreeRTOS
add_subdirectory(Lib/FreeRTOS)
# Add the CAN drivers
add_subdirectory(Drivers/CAN)
#
add_subdirectory(Lib/etl-20.38.17)
#
add_subdirectory(Lib/logger-main-c6abda2e)
#
add_subdirectory(app/)
#

include(CheckGit.cmake)
CheckGitSetup()
message(STATUS "COMMS Software version: commit $GIT_HASH")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user sources here
        Core/Src/main.c
)

target_compile_definitions(log_common PUBLIC LOGLEVEL_TRACE)

file(GLOB_RECURSE ECSS_SOURCES "Lib/ECSS-Services/src/*.cpp" "Lib/ECSS-Services/src/Helpers/*.cpp" "Lib/ECSS-Services/src/Services/*.cpp" "Lib/ECSS-Services/src/Time/*.cpp")

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
        stm32cubemx
        app_main
        # Add user defined libraries
        git_version
)


