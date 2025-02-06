#include "TCHandlingTask.hpp"
#include "RF_RXTask.hpp"
#include "Logger.hpp"

#include <ApplicationLayer.hpp>
#include <COBS.hpp>
#include <MessageParser.hpp>
#include <Peripheral_Definitions.hpp>
#include <at86rf215definitions.hpp>
#include <eMMC.hpp>
#include "Message.hpp"

#include <TPProtocol.hpp>


[[noreturn]] void TCHandlingTask::execute() {
    LOG_INFO << "TCHandlingTask::execute()";
    uint32_t received_events;
    CAN::StoredPacket TC_PACKET;
    uint8_t TC_BUFFER[2048];
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RECEIVED_TC, pdFALSE, pdTRUE, &received_events, portMAX_DELAY) == pdTRUE) {
            LOG_INFO << "RECEIVED TC";
            // TODO parse the TC
            while (uxQueueMessagesWaiting(incomingTCQueue)) {
                xQueueReceive(incomingTCQueue, &TC_PACKET, portMAX_DELAY);
                getItem(eMMC::memoryMap[eMMC::RECEIVED_TC], TC_BUFFER, 2048, TC_PACKET.pointerToeMMCItemData, 4);
                auto cobsDecodedMessage = COBSdecode<1024>(TC_BUFFER, TC_PACKET.size);
                CAN::TPMessage message = {{CAN::NodeID, CAN::OBC, false}};
                for (int i = 0; i < TC_PACKET.size; i++) {
                    message.appendUint8(TC_BUFFER[i]);
                }
                LOG_INFO << "Received message TC with length: " << TC_PACKET.size;
                if (TC_PACKET.size == 9) {
                    CAN::TPProtocol::createCANTPMessage(message, false);
                }

            }
        }
    }
}