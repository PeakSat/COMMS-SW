
add_subdirectory(Inc)
add_library(TMP117 STATIC)

target_sources(TMP117 PRIVATE Src/TMP117.cpp)

target_link_libraries(TMP117 PRIVATE TMP117_inc stm32cubemx log_common freertos_kernel app_main)

