add_subdirectory(Inc)

add_library(freertos_tasks)

target_sources(freertos_tasks PRIVATE Src/TransceiverTask.cpp Src/UARTGatekeeper.cpp)

target_include_directories(freertos_tasks PRIVATE Inc)

target_link_libraries(freertos_tasks INTERFACE freertos_tasks_headers at86rf215_inc etl::etl PRIVATE at86rf215_inc at86rf215 freertos_kernel etl::etl log_common)