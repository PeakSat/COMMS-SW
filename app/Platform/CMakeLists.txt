add_subdirectory(Inc)

add_library(platform)

target_sources(platform PRIVATE Src/PlatformParameters.cpp Src/Service.cpp Src/ErrorHandler.cpp Src/cobs.c)

target_include_directories(platform PRIVATE Inc)

target_link_libraries(platform INTERFACE platform_headers CAN_inc etl::etl PRIVATE CAN_inc CAN platform_headers freertos_kernel etl::etl log_common)