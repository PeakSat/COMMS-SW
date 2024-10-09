# FreeRTOS internal cmake file. Do not use it in user top-level project

add_library(freertos_kernel_include INTERFACE)

target_include_directories(freertos_kernel_include
    INTERFACE
        .
)

target_link_libraries(freertos_kernel_include
        INTERFACE
        freertos_config
)
