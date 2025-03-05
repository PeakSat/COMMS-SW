#include <COBS.hpp>
#include <Message.hpp>
#include <MessageParser.hpp>
#include <RF_TXTask.hpp>
#include <TMHandlingTask.hpp>
#include <at86rf215definitions.hpp>
#include <eMMC.hpp>


[[noreturn]] void TMHandling::execute() {
    LOG_INFO << "TMHandlingTask::execute()";
    TMQueue = xQueueCreateStatic(TMQueueItemNum, TMItemSize, TMQueueStorageArea,
                                           &TMQueueBuffer);
    vQueueAddToRegistry(TMQueue, "TM queue");
    while (true) {
        uint32_t received_events = 0;
        while (true) {
            if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RECEIVED_TM, pdFALSE, pdTRUE, &received_events, portMAX_DELAY) == pdTRUE) {
                if (received_events & TM_OBC_TM_HANDLING) {
                    LOG_INFO << "[TM_Handling] parsing of the TM...";
                    // TODO parse the TM
                    while (uxQueueMessagesWaiting(TMQueue)) {
                        TX_PACKET_HANDLER tm_handler{};
                        xQueueReceive(TMQueue, &tm_handler, portMAX_DELAY);
                        uint16_t new_size = 0;
                        for (int i = 0; i < tm_handler.data_length; i++) {
                            TM_BUFF[i] = tm_handler.buf[i];
                            // LOG_DEBUG << TM_BUFF[i];
                            new_size++;
                        }
                        LOG_DEBUG << "[TM_Handling] parsed TM received with parsed length: " << new_size;

                        Message message = MessageParser::parse(TM_BUFF, new_size);
                        etl::format_spec formatSpec;
                        auto serviceType = String<128>("");
                        auto messageType = String<128>("");
                        auto messageSourceId = String<128>("");
                        auto messageLength = String<128>("");
                        auto messageAPI = String<128>("");
                        auto messageApplicationId = String<128>("");
                        etl::to_string(message.serviceType, serviceType, formatSpec, false);
                        etl::to_string(message.messageType, messageType, formatSpec, false);
                        etl::to_string(message.dataSize, messageLength, formatSpec, false);
                        etl::to_string(message.applicationId, messageApplicationId, formatSpec, false);

                        auto output = String<ECSSMaxMessageSize>("[TM-HANDLING] New ");
                        if (message.packetType == Message::TM) {
                            output.append("TM[");
                        } else {
                            output.append("TC[");
                        }
                        output.append(serviceType);
                        output.append(",");
                        output.append(messageType);
                        output.append("] message!");
                        output.append(", payload length: ");
                        output.append(messageLength);
                        output.append(", API: ");
                        output.append(messageApplicationId);
                        LOG_DEBUG << output.c_str();
                        for (int i = 0 ; i < new_size; i++) {
                            tx_handler.buf[i]= TM_BUFF[i];
                        }
                        tx_handler.data_length = new_size;
                        if (message.applicationId == 1 && message.packetType == Message::TM && message.serviceType <= 23 && message.messageType <= 40 && message.dataSize < tm_handler.data_length) {
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
    }
}