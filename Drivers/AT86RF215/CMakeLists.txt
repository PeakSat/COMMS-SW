add_subdirectory(Inc)
add_library(at86rf215 STATIC)

target_sources(at86rf215 PRIVATE Src/at86rf215.cpp)

target_link_libraries(at86rf215 PUBLIC at86rf215_inc stm32cubemx freertos_kernel freertos_tasks)

