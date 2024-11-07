#include "CANTestTask.hpp"
#include "ApplicationLayer.hpp"
#include "CANGatekeeperTask.hpp"

void CANTestTask::execute() {

    CAN::CANBuffer_t message = {};
    /**
     * Simple 64 byte message sending
     */

    for (uint8_t idx = 0; idx < CAN::Frame::MaxDataLength; idx++) {
        message.push_back(idx);
    }
    String<ECSSMaxMessageSize> testPayload1("CAN1 SAYS: SPONGEBOB SQUAREPANTS! ");

    String<ECSSMaxMessageSize> testPayload2("CAN2 SAYS: WHO LET THE DOGS OUT!?");
    CAN::ActiveBus activeBus = CAN::ActiveBus::Redundant;
    while (true) {
        if(activeBus == CAN::ActiveBus::Redundant) {
            activeBus = CAN::ActiveBus::Main;
            canGatekeeperTask->switchActiveBus(activeBus);
            CAN::Application::createLogMessage(CAN::NodeIDs::OBC, false, testPayload1.data(), false);
            LOG_DEBUG << "REDUNDANT CAN is sending";
        } else {
            activeBus = CAN::ActiveBus::Redundant;
            canGatekeeperTask->switchActiveBus(activeBus);
            CAN::Application::createLogMessage(CAN::NodeIDs::OBC, false, testPayload2.data(), false);
            LOG_DEBUG << "MAIN CAN is sending";
        }
        xTaskNotify(canGatekeeperTask->taskHandle, 0, eNoAction);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

}