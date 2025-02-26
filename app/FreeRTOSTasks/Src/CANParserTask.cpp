#include "CANParserTask.hpp"
#include "ApplicationLayer.hpp"
#include "CANGatekeeperTask.hpp"

#include <TPProtocol.hpp>
#include <eMMC.hpp>

void CANParserTask::execute() {

    uint32_t eMMCPacketTailPointer = 0;
    int counter = 0;
    uint32_t ulNotifiedValue;
    while (true) {

        xTaskNotifyWait(0, 0, &ulNotifiedValue, pdMS_TO_TICKS(1000));
        while (uxQueueMessagesWaiting(canGatekeeperTask->storedPacketQueue)) {

            // Get packet from eMMC
            CAN::StoredPacket StoredPacket;
            xQueueReceive(canGatekeeperTask->storedPacketQueue, &StoredPacket, portMAX_DELAY);

            struct localPacketHandler CANPacketHandler;

            memoryQueueItemHandler dequeueHandler{};
            xQueueReceive(testDataQueue, &dequeueHandler, portMAX_DELAY);
            eMMC::getItemFromQueue(eMMC::memoryQueueMap[eMMC::testData], dequeueHandler, reinterpret_cast<uint8_t*>(&CANPacketHandler), dequeueHandler.size);


            if (uxQueueMessagesWaiting(canGatekeeperTask->storedPacketQueue) == 0) {

                // Get packet from eMMC
                uint8_t messageBuff[1024];

                CAN::Application::getStoredMessage(&StoredPacket, messageBuff, StoredPacket.size, sizeof(messageBuff) / sizeof(messageBuff[0]));
                LOG_DEBUG << "INCOMING CAN MESSAGE OF SIZE: " << StoredPacket.size;

                // Send ACK
                CAN::TPMessage ACKmessage = {{CAN::NodeID, CAN::NodeIDs::OBC, false}};
                ACKmessage.appendUint8(CAN::Application::MessageIDs::ACK);
                CAN::TPProtocol::createCANTPMessageNoRetransmit(ACKmessage, false);
                xTaskNotifyGive(canGatekeeperTask->taskHandle);

                // parse

                uint8_t messageID = static_cast<CAN::Application::MessageIDs>(StoredPacket.Identifier);
                if (messageID == CAN::Application::CCSDSPacket) {
                    xQueueSendToBack(TXQueue, &StoredPacket, NULL);
                    xTaskNotifyIndexed(rf_txtask->taskHandle, NOTIFY_INDEX_TRANSMIT, TM_OBC, eSetBits);
                    LOG_DEBUG << "New TM received: ";
                } else {
                    CAN::TPMessage message;
                    message.appendUint8(StoredPacket.Identifier);
                    for (int i = 0; i < StoredPacket.size; i++) {
                        message.appendUint8(messageBuff[i]);
                    }
                    message.idInfo.sourceAddress = CAN::OBC;
                    CAN::TPProtocol::parseMessage(message);
                }

            } else {
                LOG_DEBUG << "Old message discarded";
            }
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