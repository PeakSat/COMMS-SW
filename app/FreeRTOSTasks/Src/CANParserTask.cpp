#include "CANParserTask.hpp"
#include "ApplicationLayer.hpp"
#include "CANGatekeeperTask.hpp"

#include <TCHandlingTask.hpp>
#include <TMHandlingTask.hpp>
#include <TPProtocol.hpp>
#include <eMMC.hpp>

void CANParserTask::execute() {

    uint32_t ulNotifiedValue;
    while (true) {

        xTaskNotifyWait(0, 0, &ulNotifiedValue, pdMS_TO_TICKS(1000));
        while (uxQueueMessagesWaiting(canGatekeeperTask->storedPacketQueue)) {

            // Get packet from eMMC
            CAN::StoredPacket StoredPacket{};
            xQueueReceive(canGatekeeperTask->storedPacketQueue, &StoredPacket, portMAX_DELAY);

            if (uxQueueMessagesWaiting(canGatekeeperTask->storedPacketQueue) == 0) {

                // Get packet from eMMC
                uint8_t messageBuff[1024]{};
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
                    for (int i = 0 ; i < StoredPacket.size ; i++) {
                        TX_BUF_CAN[i] = messageBuff[i];
                        // LOG_DEBUG << "[CAN-PARSER] TM DATA: " << messageBuff[i];
                    }

                    tm_handler.pointer_to_data = TX_BUF_CAN;
                    tm_handler.data_length = StoredPacket.size;
                    xQueueSendToBack(TMQueue, &tm_handler, NULL);
                    if (tmhandlingTask->taskHandle != nullptr) {
                        xTaskNotifyIndexed(tmhandlingTask->taskHandle, NOTIFY_INDEX_RECEIVED_TM, TM_OBC_TM_HANDLING, eSetBits);
                    }
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
    }
}