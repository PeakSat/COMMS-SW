#include "CANParserTask.hpp"
#include "ApplicationLayer.hpp"
#include "CANGatekeeperTask.hpp"

#include <TPProtocol.hpp>
#include <eMMC.hpp>

void CANParserTask::execute() {

    // CAN::CANBuffer_t message = {};
    // /**
    //  * Simple 64 byte message sending
    //  */
    //
    // for (uint8_t idx = 0; idx < CAN::MaxPayloadLength; idx++) {
    //     message.push_back(idx);
    // }
    // String<ECSSMaxMessageSize> testPayload1("ccccccccccccccccccccccccccccccccc");
    //
    // String<ECSSMaxMessageSize> testPayload2("d");
    //
    // CAN::ActiveBus activeBus = CAN::ActiveBus::Redundant;

    uint32_t eMMCPacketTailPointer = 0;
    int counter = 0;
    uint32_t ulNotifiedValue;
    while (true) {

        xTaskNotifyWait(0, 0, &ulNotifiedValue, pdMS_TO_TICKS(1000));
        vTaskDelay(10);
        while (uxQueueMessagesWaiting(canGatekeeperTask->storedPacketQueue)) {

            // Get packet from eMMC
            CAN::StoredPacket StoredPacket;
            xQueueReceive(canGatekeeperTask->storedPacketQueue, &StoredPacket, portMAX_DELAY);
            uint8_t messageBuff[1024];
            CAN::Application::getStoredMessage(&StoredPacket, messageBuff, StoredPacket.size, sizeof(messageBuff) / sizeof(messageBuff[0]));
            LOG_DEBUG << "INCOMING CAN MESSAGE OF SIZE: " << StoredPacket.size;

            // parse
            CAN::TPMessage message;
            message.appendUint8(StoredPacket.Identifier);
            for (int i = 0; i < StoredPacket.size; i++) {
                message.appendUint8(messageBuff[i]);
            }
            message.idInfo.sourceAddress = CAN::OBC;
            uint32_t start = xTaskGetTickCount();
            CAN::TPProtocol::parseMessage(message);


            //Send ACK
            CAN::TPMessage ACKmessage = {{CAN::NodeID, CAN::NodeIDs::OBC, false}};
            ACKmessage.appendUint8(CAN::Application::MessageIDs::ACK);
            CAN::TPProtocol::createCANTPMessageNoRetransmit(ACKmessage, false);
            xTaskNotifyGive(canGatekeeperTask->taskHandle);
        }


        // if (counter == 0) {
        //     counter = 1;
        //     // CAN::Application::createLogMessage(CAN::NodeIDs::OBC, false, testPayload1.data(), false);
        //     CAN::Application::createLogMessage(CAN::NodeIDs::OBC, false, testPayload1.data(), false);
        //
        //     LOG_DEBUG << "REDUNDANT CAN is sending";
        // } else {
        //     counter = 0;
        //     // CAN::Application::createLogMessage(CAN::NodeIDs::OBC, false, testPayload2.data(), false);
        //     CAN::Application::createLogMessage(CAN::NodeIDs::OBC, false, testPayload2.data(), false);
        //
        //     LOG_DEBUG << "MAIN CAN is sending";
        // }
        // while (uxQueueMessagesWaiting(canGatekeeperTask->storedPacketQueue)) {
        //     CAN::StoredPacket StoredPacket;
        //     xQueueReceive(canGatekeeperTask->storedPacketQueue, &StoredPacket, portMAX_DELAY);
        //     uint8_t messageBuff[StoredPacket.size];
        //     CAN::Application::getStoredMessage(&StoredPacket, messageBuff, StoredPacket.size, sizeof(messageBuff) / sizeof(messageBuff[0]));
        //     LOG_DEBUG << "INCOMING CAN MESSAGE OF SIZE: " << StoredPacket.size;
        // }
        // vTaskDelay(3000);
    }
}