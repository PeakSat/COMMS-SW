#include "CANTestTask.hpp"
#include "ApplicationLayer.hpp"
#include "CANGatekeeperTask.hpp"

#include <eMMC.hpp>

void CANTestTask::execute() {

    CAN::CANBuffer_t message = {};
    /**
     * Simple 64 byte message sending
     */

    for (uint8_t idx = 0; idx < CAN::MaxPayloadLength; idx++) {
        message.push_back(idx);
    }
    String<ECSSMaxMessageSize> testPayload1("CAN1 SAYS: SPONGEBOB SQUAREPANTS! ");

    String<ECSSMaxMessageSize> testPayload2("CAN2 SAYS: WHO LET THE DOGS OUT!?");
    CAN::ActiveBus activeBus = CAN::ActiveBus::Redundant;
    while (true) {
        if (activeBus == CAN::ActiveBus::Redundant) {
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
        while (uxQueueMessagesWaiting(canGatekeeperTask->storedPacketQueue)) {
            CAN::StoredPacket StoredPacket;
            xQueueReceive(canGatekeeperTask->storedPacketQueue, &StoredPacket, portMAX_DELAY);
            uint8_t messageBuff[StoredPacket.size];
            CAN::Application::getStoredMessage(&StoredPacket, messageBuff, StoredPacket.size, sizeof(messageBuff) / sizeof(messageBuff[0]));
            LOG_DEBUG << "INCOMING CAN MESSAGE OF SIZE: " << StoredPacket.size;
        }
        vTaskDelay(3000);
    }
}