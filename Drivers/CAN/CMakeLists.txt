add_subdirectory(Inc)
add_library(CAN STATIC)

target_sources(CAN PUBLIC Src/CANDriver.cpp Src/TPProtocol.cpp)

target_link_libraries(CAN PUBLIC etl CAN_inc stm32cubemx log_common freertos_tasks)