add_subdirectory(Inc)
add_library(INA3221 STATIC)

target_sources(INA3221 PRIVATE Src/INA3221.cpp)

target_link_libraries(INA3221 PRIVATE INA3221_inc stm32cubemx log_common freertos_kernel app_main)
