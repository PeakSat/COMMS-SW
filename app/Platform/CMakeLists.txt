add_subdirectory(Inc)

add_library(platform
        Src/FunctionManagementWrappers.cpp)

target_sources(platform PUBLIC Src/PlatformParameters.cpp Src/ErrorHandlerTemplates.cpp Src/cobs.c Src/FunctionManagementWrappers.cpp)

target_include_directories(platform PRIVATE Inc)

target_link_libraries(platform INTERFACE platform_headers CAN_inc gnss_inc etl::etl PRIVATE CAN_inc CAN gnss_inc gnss platform_headers freertos_kernel etl::etl log_common)