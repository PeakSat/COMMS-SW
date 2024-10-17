add_subdirectory(Inc)

add_library(freertos_tasks)

target_sources(freertos_tasks PRIVATE Src/TransceiverTask.cpp Src/UARTGatekeeper.cpp Src/eMMCTask.cpp Src/GNSSTask.cpp)

target_include_directories(freertos_tasks PRIVATE Inc)

target_link_libraries(freertos_tasks INTERFACE freertos_tasks_headers at86rf215_inc eMMC_inc etl::etl gnss_inc PRIVATE at86rf215_inc at86rf215 eMMC_inc eMMC gnss_inc gnss freertos_kernel etl::etl log_common )