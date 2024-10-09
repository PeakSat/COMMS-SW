cmake_minimum_required(VERSION 3.14)
project(Logger)
set(CMAKE_CXX_STANDARD 17)

add_library(log_common STATIC)
target_include_directories(log_common PUBLIC "inc/")
target_sources(log_common PRIVATE "src/Logger.cpp")
target_link_libraries(log_common PUBLIC "etl::etl")
target_link_libraries(log_common PRIVATE freertos_kernel)
target_link_libraries(log_common PRIVATE freertos_tasks_headers)


add_library(log_x86 STATIC)
target_sources(log_x86 PRIVATE "src/Platform/x86/Logger.cpp")
target_link_libraries(log_x86 PRIVATE "etl::etl")
target_link_libraries(log_x86 PRIVATE "log_common")

add_library(Logger::x86 ALIAS "log_x86")

add_executable(Logger)
target_sources(Logger PRIVATE src/main.cpp)
target_link_libraries(Logger PRIVATE "log_x86")

