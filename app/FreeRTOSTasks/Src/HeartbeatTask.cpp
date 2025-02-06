#include "Logger.hpp"
#include "HeartbeatTask.hpp"
#include "PlatformParameters.hpp"

#include <ApplicationLayer.hpp>

void HeartbeatTask::execute() {
    LOG_DEBUG << "HeartbeatTask::execute()";
    vTaskDelay(pdMS_TO_TICKS(500));
    CAN::Application::sendHeartbeatMessage();
    uint32_t repetitionCounter = 0;
    uint32_t falseCounter = 0;

    while (true) {
        // LOG_DEBUG << "HeartbeatTask::execute()";
        if (heartbeatReceived == true) {
            heartbeatReceived = false;
            falseCounter = 0;
        } else if (falseCounter == 5) {
            LOG_ERROR << "Heartbeat timeout";
            // todo: Maybe reset OBC at some point
        }
        if (falseCounter < 6) {
            falseCounter++;
        }

        if (repetitionCounter++ > 2) {
            repetitionCounter = 0;
            CAN::Application::sendHeartbeatMessage();
        }

        vTaskDelay(pdMS_TO_TICKS(OBDHParameters::HeartbeatPeriod.getValue() / 3));
    }
}
