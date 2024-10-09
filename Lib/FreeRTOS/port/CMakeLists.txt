# ARM_CM7 port
add_library(freertos_kernel_port OBJECT)

add_library(freertos_kernel_port_headers INTERFACE)

target_include_directories(freertos_kernel_port_headers INTERFACE .)

target_sources(freertos_kernel_port PRIVATE
        port.c
        MemMang/heap_4.c
)

target_link_libraries(freertos_kernel_port
        PUBLIC
        freertos_kernel_port_headers
        PRIVATE
        freertos_kernel_include)
