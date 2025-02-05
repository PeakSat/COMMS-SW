#include "TCHandlingTask.hpp"
#include "Logger.hpp"

[[noreturn]] void TCHandlingTask::execute() {
    LOG_INFO << "TCHandlingTask::execute()";
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}