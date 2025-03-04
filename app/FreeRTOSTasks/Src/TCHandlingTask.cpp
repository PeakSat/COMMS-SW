#include "TCHandlingTask.hpp"
#include "RF_RXTask.hpp"
#include "RF_TXTask.hpp"
#include "Logger.hpp"
#include <ApplicationLayer.hpp>
#include <COBS.hpp>
#include <MessageParser.hpp>
#include <Peripheral_Definitions.hpp>
#include <at86rf215definitions.hpp>
#include <eMMC.hpp>
#include "Message.hpp"
#include <TPProtocol.hpp>
#include <cstring>

void TCHandlingTask::startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size) {
    // start the DMA
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, buf, size);
    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
    //  disabling the full buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_TC);
}

[[noreturn]] void TCHandlingTask::execute() {
    /// TODO: Be sure that the memory is available (eMMC)
    vTaskDelay(pdMS_TO_TICKS(2500));
    tc_buf_dma_pointer = TC_UART_BUF;
    startReceiveFromUARTwithIdle(tc_buf_dma_pointer, 1024);
    uint32_t received_events_tc = 0;
    TCUARTQueueHandle = xQueueCreateStatic(TCUARTQueueSize, TCUARTItemSize, incomingTCUARTQueueStorageArea,
                                        &incomingTCUARTQueueBuffer);
    vQueueAddToRegistry(TCUARTQueueHandle, "TC UART queue");

    LOG_INFO << "TCHandlingTask::execute()";
    uint16_t new_size = 0;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_INCOMING_TC, pdFALSE, pdTRUE, &received_events_tc, portMAX_DELAY) == pdTRUE) {
            if ((received_events_tc & TC_RF_RX) && tc_rf_rx_var) {
                tc_rf_rx_var = false;
                /// TODO: parse and check if it is for our spacecraft before sending it to the OBC...For example check the spacecraft id and the hmac and the length of the packet (because AT may has received less or more bytes than expected)
                LOG_INFO << "Parsing of the incoming TC from RX side...";
                while (uxQueueMessagesWaiting(rf_rx_tcQueue)) {
                    memoryQueueItemHandler rf_rx_tx_queue_handler{};
                    xQueueReceive(rf_rx_tcQueue, &rf_rx_tx_queue_handler, portMAX_DELAY);
                    uint8_t local_tc_rx_bf[1024]{};
                    eMMC::getItemFromQueue(eMMC::memoryQueueMap[eMMC::rf_rx_tc], rf_rx_tx_queue_handler, local_tc_rx_bf, rf_rx_tx_queue_handler.size);
                    new_size = 0;
                    for (int i = 0; i < rf_rx_tx_queue_handler.size; i++) {
                        ECSS_TC_BUF[i] = local_tc_rx_bf[i];
                        new_size++;
                    }
                    Message message = MessageParser::parse(ECSS_TC_BUF, new_size);
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

                    auto output = String<ECSSMaxMessageSize>("[RX SIDE] New ");
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
                    // TODO
                    new_size = 0;
                    if (message.applicationId == OBC_APPLICATION_ID &&
                        message.packetType == Message::TC &&
                        (message.serviceType == HOUSEKEEPING || message.serviceType == FUNCTION_MANAGEMENT || message.serviceType == TEST) &&
                        message.messageType <= 40 &&
                        message.dataSize < (rf_rx_tx_queue_handler.size - 5)) {
                        for (int i = 5; i < rf_rx_tx_queue_handler.size; i++) {
                            ECSS_TC_BUF[i-5] = local_tc_rx_bf[i];
                            new_size++;
                        }
                        auto cobsDecodedMessage = COBSdecode<512>(ECSS_TC_BUF, new_size);
                        LOG_INFO << "Transmitting the TC to the OBC through CAN...";
                        CAN::Application::createPacketMessage(CAN::OBC, false, cobsDecodedMessage,  Message::TC, false);
                        received_events_tc &= ~TC_RF_RX;
                    }
                    else {
                        LOG_ERROR << "[TCHANDLING - FROM RX] Received an invalid message type!";
                    }
                }
            }
            if ((received_events_tc & TC_UART) && tc_uart_var) {
                tc_uart_var = false;
                uint8_t* local_tc_uart_bf;
                LOG_INFO << "parsing of the incoming TC from UART side...";
                while (uxQueueMessagesWaiting(TCUARTQueueHandle)) {
                    if (xQueueReceive(TCUARTQueueHandle, &local_tc_uart_bf, pdMS_TO_TICKS(1000)) == pdTRUE) {
                        new_size = 0;
                        for (uint16_t i = 1; i < size-1; i++) {
                            LOG_DEBUG << "Received TC data: " << local_tc_uart_bf[i];
                            ECSS_TC_BUF[i-1] = local_tc_uart_bf[i];
                            new_size++;
                        }
                        // For some reason the script puts a 5 on the 4th pos, so we make it zero
                        ECSS_TC_BUF[4] = 0;
                        LOG_DEBUG << "RECEIVED TC FROM UART-YAMCS, size: " << new_size;
                        if(ECSS_TC_BUF[1] == Message::TC) {
                            // Parse the message
                            Message message = MessageParser::parse(ECSS_TC_BUF, new_size);
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

                            auto output = String<ECSSMaxMessageSize>("New ");
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
                            if (message.applicationId == OBC_APPLICATION_ID) {
                                LOG_DEBUG << "Received TC from UART destined for OBC";
                                tx_handler.pointer_to_data = ECSS_TC_BUF;
                                tx_handler.data_length = new_size;
                                xQueueSendToBack(TXQueue, &tx_handler, 0);
                                if (rf_txtask->taskHandle != nullptr) {
                                    xTaskNotifyIndexed(rf_txtask->taskHandle, NOTIFY_INDEX_TRANSMIT, TC_UART_TC_HANDLING_TASK, eSetBits);
                                }
                                else
                                    LOG_ERROR << "TASK HANDLE NULL";
                            }
                            else if (message.applicationId == COMMS_APPLICATION_ID) {
                                /// TODO: Forward the TC to the COMMS Execution Task
                                LOG_DEBUG << "Received TC from UART destined for COMMS";
                            }
                        }
                        else {
                            LOG_ERROR << "Message is not a TC message";
                        }
                        xQueueReset(TCUARTQueueHandle);
                    }
                }
            }
        }
    }
}