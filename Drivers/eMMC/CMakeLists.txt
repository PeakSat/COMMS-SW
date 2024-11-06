add_subdirectory(Inc)
add_library(eMMC STATIC)

target_sources(eMMC PRIVATE Src/eMMC.cpp)

target_link_libraries(eMMC PRIVATE eMMC_inc stm32cubemx log_common freertos_kernel app_main)
