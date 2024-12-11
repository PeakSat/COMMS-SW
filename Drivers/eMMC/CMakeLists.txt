add_subdirectory(Inc)
add_library(eMMC STATIC)

target_sources(eMMC PRIVATE Src/eMMC.cpp)

target_link_libraries(eMMC PUBLIC eMMC_inc stm32cubemx freertos_kernel app_main etl::etl)

