add_subdirectory(config)
add_subdirectory(inc)
add_subdirectory(port)

add_library(freertos_kernel STATIC)

target_sources(freertos_kernel PRIVATE
        src/croutine.c
        src/event_groups.c
        src/list.c
        src/queue.c
        src/stream_buffer.c
        src/tasks.c
        src/timers.c
)
target_link_libraries(freertos_kernel
        PUBLIC
        freertos_kernel_include
        freertos_kernel_port_headers
        PRIVATE
        freertos_kernel_port
)