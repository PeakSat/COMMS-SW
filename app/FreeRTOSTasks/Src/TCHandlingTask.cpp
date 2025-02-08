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
#include <cstring>  // For memcpy
#include <TPProtocol.hpp>


[[noreturn]] void TCHandlingTask::execute() {
    LOG_INFO << "TCHandlingTask::execute()";
    uint32_t received_events;
    CAN::StoredPacket TC_PACKET;
    uint8_t ecss_buf[512];
    uint8_t counter = 0;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RECEIVED_TC, pdFALSE, pdTRUE, &received_events, portMAX_DELAY) == pdTRUE) {
            LOG_INFO << "parsing of the TC...";
            // TODO parse the TC
            while (uxQueueMessagesWaiting(incomingTCQueue)) {
                uint8_t TC_BUFFER[512];
                xQueueReceive(incomingTCQueue, &TC_PACKET, portMAX_DELAY);
                getItem(eMMC::memoryMap[eMMC::RECEIVED_TC], TC_BUFFER, 512, TC_PACKET.pointerToeMMCItemData, 1);
                for (uint8_t i = 6; i < TC_PACKET.size; i++) {
                    counter++;
                    ecss_buf[i - 6] = TC_BUFFER[i];
                }
                auto cobsDecodedMessage = COBSdecode<1024>(ecss_buf, TC_PACKET.size - 6);
                CAN::Application::createPacketMessage(CAN::OBC, false, cobsDecodedMessage,  Message::TC, false);
            }
        }
    }
}