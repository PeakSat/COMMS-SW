add_subdirectory(Inc)

add_library(freertos_tasks)

target_sources(freertos_tasks PRIVATE Src/TransceiverTask.cpp Src/UARTGatekeeper.cpp Src/eMMCTask.cpp Src/INA3221Task.cpp Src/CANGatekeeperTask.cpp Src/TMP117Task.cpp)

target_include_directories(freertos_tasks PRIVATE Inc)

target_link_libraries(freertos_tasks INTERFACE freertos_tasks_headers at86rf215_inc eMMC_inc INA3221_inc etl::etl CAN_inc TMP117_inc PRIVATE at86rf215_inc at86rf215 eMMC_inc eMMC INA3221_inc INA3221 CAN_inc CAN TMP117_inc TMP117 freertos_kernel freertos_tasks_headers etl::etl log_common )