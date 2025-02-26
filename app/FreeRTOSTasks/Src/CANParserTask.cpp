#include "CANParserTask.hpp"
#include "ApplicationLayer.hpp"
#include "CANGatekeeperTask.hpp"

#include <TPProtocol.hpp>
#include <eMMC.hpp>

void CANParserTask::execute() {

    uint32_t ulNotifiedValue;
    while (true) {

        xTaskNotifyWait(0, 0, &ulNotifiedValue, pdMS_TO_TICKS(1000));
        while (uxQueueMessagesWaiting(CANRxQueue)) {

            struct localPacketHandler CANPacketHandler;
            memoryQueueItemHandler dequeueHandler{};
            xQueueReceive(CANRxQueue, &dequeueHandler, portMAX_DELAY);
            eMMC::getItemFromQueue(eMMC::memoryQueueMap[eMMC::testData], dequeueHandler, reinterpret_cast<uint8_t*>(&CANPacketHandler), dequeueHandler.size);


            if (uxQueueMessagesWaiting(CANRxQueue) == 0) {

                // Send ACK
                CAN::TPMessage ACKmessage = {{CAN::NodeID, CAN::NodeIDs::OBC, false}};
                ACKmessage.appendUint8(CAN::Application::MessageIDs::ACK);
                CAN::TPProtocol::createCANTPMessageNoRetransmit(ACKmessage, false);
                xTaskNotifyGive(canGatekeeperTask->taskHandle);

                // parse

                uint8_t messageID = static_cast<CAN::Application::MessageIDs>(CANPacketHandler.MessageID);
                if (messageID == CAN::Application::CCSDSPacket) {
                    // todo: change rf queue
                    LOG_DEBUG << "New TM received: ";
                } else {
                    CAN::TPMessage message;
                    message.appendUint8(CANPacketHandler.MessageID);
                    for (int i = 0; i < CANPacketHandler.PacketSize; i++) {
                        message.appendUint8(CANPacketHandler.Buffer[i]);
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