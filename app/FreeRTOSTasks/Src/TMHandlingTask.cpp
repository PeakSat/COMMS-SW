#pragma once
#include <MessageParser.hpp>
#include <RF_TXTask.hpp>
#include <TMHandlingTask.hpp>
#include <TCHandlingTask.hpp>


[[noreturn]] void TMHandling::execute() {
    LOG_INFO << "TMHandlingTask::execute()";
    TMQueue = xQueueCreateStatic(TMQueueItemNum, TMItemSize, TMQueueStorageArea, &TMQueueBuffer);
    vQueueAddToRegistry(TMQueue, "TM queue");

    while (true) {
        uint32_t received_events = 0;
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RECEIVED_TM, pdFALSE, pdTRUE, &received_events, portMAX_DELAY) == pdTRUE) {
            LOG_INFO << "[TM_Handling] parsing of the TM...";
            while (uxQueueMessagesWaiting(TMQueue)) {
                TX_PACKET_HANDLER tm_handler{};
                xQueueReceive(TMQueue, &tm_handler, portMAX_DELAY);
                uint16_t new_size = 0;
                for (int i = 0; i < tm_handler.data_length; i++) {
                    TM_BUFF[i] = tm_handler.buf[i];
                    new_size++;
                }
                LOG_DEBUG << "[TM_Handling] parsed TM received with parsed length: " << new_size;
                Message message = MessageParser::parse(TM_BUFF, new_size);
                tcHandlingTask->logParsedMessage(message);
                TX_PACKET_HANDLER tx_handler{};
                for (int i = 0; i < new_size; i++) {
                    tx_handler.buf[i] = TM_BUFF[i];
                }
                tx_handler.data_length = new_size;
                if (message.packetType == Message::TM && message.serviceType <= 23 && message.messageType <= 40 && message.dataSize < tm_handler.data_length) {
                    xQueueSendToBack(TXQueue, &tx_handler, NULL);
                    if (rf_txtask->taskHandle != nullptr) {
                        xTaskNotifyIndexed(rf_txtask->taskHandle, NOTIFY_INDEX_TRANSMIT, TM_OBC, eSetBits);
                    }
                }
            }
            xQueueReset(TMQueue);
        }
    }
}