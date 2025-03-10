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
set(CMAKE_PROJECT_NAME ttc-flight-software)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

message("Build type: " ${CMAKE_BUILD_TYPE})

include_directories(
        Lib/ecss-services/inc
        Lib/ecss-services/inc/etl
        Lib/ecss-services/inc/Helpers
        Lib/ecss-services/inc/Services
        Lib/ecss-services/inc/Time
        Lib/etl-20.38.17/include
        Lib/logger-main-c6abda2e/inc
)

# Create an executable object type
add_executable(
        ${CMAKE_PROJECT_NAME}
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
add_subdirectory(app/Platform)

add_subdirectory(Lib/ecss-services)

include(CheckGit.cmake)
CheckGitSetup()
message(STATUS "TTC Software version: commit $GIT_HASH")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user sources here
        Core/Src/main.c
)

# Set the Python executable explicitly
set(Python3_EXECUTABLE "python3")

# Try to find Python again
find_package(Python3 REQUIRED)


# Find Python 3 and specify the version you want
find_package(Python3 3.6 REQUIRED)

# Display the Python version found by CMake
message(STATUS "Found Python: ${Python3_EXECUTABLE} (${Python3_VERSION})")


# Set up the virtual environment path
set(VENV_PATH "${CMAKE_SOURCE_DIR}/.venv")

# Check if virtual environment exists
if(NOT EXISTS "${VENV_PATH}/bin/activate")
    message(STATUS "Creating virtual environment at ${VENV_PATH}")

    # Create the virtual environment
    execute_process(
            COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_PATH}
            RESULT_VARIABLE result
            OUTPUT_VARIABLE result_output
            ERROR_VARIABLE result_error
            ERROR_QUIET
    )

    if(result)
        message(FATAL_ERROR "Failed to create virtual environment. Error: ${result_error}")
    else()
        message(STATUS "Virtual environment created successfully")
    endif()

    # Show stdout and stderr
    message(STATUS "Standard Output: ${result_output}")
    message(STATUS "Standard Error: ${result_error}")

    # Install dependencies using pip (requirements.txt should exist)
    execute_process(
            COMMAND ${VENV_PATH}/bin/pip install --upgrade pip
            COMMAND ${VENV_PATH}/bin/pip install -r ${CMAKE_SOURCE_DIR}/requirements.txt
            RESULT_VARIABLE install_result
            ERROR_QUIET
    )

    if(install_result)
        message(FATAL_ERROR "Failed to install dependencies from requirements.txt")
    endif()
else()
    message(STATUS "Virtual environment already exists at ${VENV_PATH}")
endif()

# Run your Python script using the virtual environment's Python interpreter
execute_process(
        COMMAND ${VENV_PATH}/bin/python ${CMAKE_SOURCE_DIR}/app/Platform/update_database.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_compile_definitions(ttc-flight-software)

target_compile_definitions(log_common PUBLIC LOGLEVEL_TRACE)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
        ecss
        stm32cubemx
        app_main
        # Add user defined libraries
        git_version
)


