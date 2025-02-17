add_subdirectory(Inc)

add_library(freertos_tasks)

target_sources(freertos_tasks PRIVATE  Src/UARTGatekeeper.cpp Src/eMMCTask.cpp Src/TestTask.cpp Src/INA3221Task.cpp Src/TMP117Task.cpp Src/GNSSTask.cpp Src/CANGatekeeperTask.cpp Src/CANParserTask.cpp Src/RF_TXTask.cpp Src/RF_RXTask.cpp Src/TCHandlingTask.cpp Src/TMParserTask.cpp Src/HeartbeatTask.cpp)

target_include_directories(freertos_tasks PRIVATE Inc)

target_link_libraries(freertos_tasks INTERFACE freertos_tasks_headers at86rf215_inc eMMC_inc CAN_inc TMP117_inc INA3221_inc etl::etl gnss_inc PRIVATE at86rf215_inc at86rf215 eMMC_inc eMMC gnss_inc gnss CAN_inc CAN TMP117_inc TMP117 INA3221_inc INA3221 freertos_kernel etl::etl log_common )