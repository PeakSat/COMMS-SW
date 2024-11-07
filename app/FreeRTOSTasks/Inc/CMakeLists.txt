add_library(freertos_tasks_headers INTERFACE
        CANTestTask.hpp)
target_include_directories(freertos_tasks_headers INTERFACE .)
target_link_libraries(freertos_tasks_headers INTERFACE freertos_kernel)