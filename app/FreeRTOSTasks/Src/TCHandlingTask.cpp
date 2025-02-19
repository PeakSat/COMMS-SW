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
    vTaskDelay(6000);
    tc_buf_dma_pointer = TC_BUF;
    startReceiveFromUARTwithIdle(tc_buf_dma_pointer, 512);
    uint32_t received_events;
    TCUARTQueueHandle = xQueueCreateStatic(TCUARTQueueSize, sizeof(uint8_t*), incomingTCUARTQueueStorageArea,
                                        &incomingTCQueueBuffer);
    vQueueAddToRegistry(TCUARTQueueHandle, "TC UART queue");

    LOG_INFO << "TCHandlingTask::execute()";
    CAN::StoredPacket TC_PACKET;
    uint8_t* tc_buf_from_queue_pointer;
    uint8_t ECSS_TC_BUF[1024] = {};
    uint8_t TC_BUFFER[1024] = {};
    uint32_t eMMCPacketTailPointer = 0;
    uint16_t new_size = 0;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_INCOMING_TC, pdFALSE, pdTRUE, &received_events, portMAX_DELAY) == pdTRUE) {
            if (received_events & TC_RF_RX) {
                /// TODO: parse and check if it is for our spacecraft before sending it to the OBC...For example check the spacecraft id and the hmac and the length of the packet (because AT may has received less or more bytes than expected)
                LOG_INFO << "parsing of the incoming TC from RX side...";
                while (uxQueueMessagesWaiting(incomingTCQueue)) {
                    xQueueReceive(incomingTCQueue, &TC_PACKET, portMAX_DELAY);
                    getItem(eMMC::memoryMap[eMMC::RX_TC], TC_BUFFER, 1024, TC_PACKET.pointerToeMMCItemData, 2);
                    CAN::TPMessage message = {{CAN::NodeID, CAN::OBC, false}};
                    /// TODO: we have to find the correct CCSDS headers and abort the magic number solution
                    for (int i = 6; i < TC_PACKET.size; i++) {
                        ECSS_TC_BUF[i-6] = TC_BUFFER[i];
                    }
                    auto cobsDecodedMessage = COBSdecode<512>(ECSS_TC_BUF, TC_PACKET.size - 6);
                    CAN::Application::createPacketMessage(CAN::OBC, false, cobsDecodedMessage,  Message::TC, false);
                }
            }
            if (received_events & TC_UART) {
                LOG_INFO << "parsing of the incoming TC from UART side...";
                if (xQueueReceive(TCUARTQueueHandle, &tc_buf_from_queue_pointer, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    if (tc_buf_from_queue_pointer != nullptr) {
                        new_size = 0;
                        for (uint16_t i = 1; i < size-1; i++) {
                            LOG_DEBUG << "Received TC data: " << tc_buf_from_queue_pointer[i];
                            ECSS_TC_BUF[i-1] = tc_buf_from_queue_pointer[i];
                            new_size++;
                        }
                        // For some reason the script puts a 5 on the 4th pos, so we make it zero
                        ECSS_TC_BUF[4] = 0;
                        LOG_DEBUG << "RECEIVED TC FROM UART-YAMCS, size: " << new_size;
                        if(new_size >= 9) {
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
                            // etl::to_string(message.sourceId, messageSourceId, formatSpec, false);
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
                            output.append(", with paylod length:");
                            output.append(messageLength);
                            output.append(", for API: ");
                            output.append(messageApplicationId);
                            // output.append(", from sourceId: ");
                            // output.append(messageSourceId);
                            LOG_DEBUG << output.c_str();
                            if (ECSS_TC_BUF[1] == OBC_APPLICATION_ID) {
                                LOG_DEBUG << "Received TC from UART destined for OBC";
                                auto status = storeItem(eMMC::memoryMap[eMMC::UART_TC], ECSS_TC_BUF, 1024, eMMCPacketTailPointer, 2);
                                if (status.has_value()) {
                                    CAN::StoredPacket PacketToBeStored;
                                    PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
                                    eMMCPacketTailPointer += 2;
                                    PacketToBeStored.size = new_size;
                                    xQueueSendToBack(TXQueue, &PacketToBeStored, 0);
                                    if (rf_txtask->taskHandle != nullptr) {
                                        xTaskNotifyIndexed(rf_txtask->taskHandle, NOTIFY_INDEX_TRANSMIT, TC_UART_TC_HANDLING_TASK, eSetBits);
                                    }
                                    else
                                        LOG_ERROR << "TASK HANDLE NULL";
                                }
                                else {
                                    LOG_ERROR << "MEMORY ERROR ON TC";
                                }
                            }
                            else if (ECSS_TC_BUF[1] == COMMS_APPLICATION_ID) {
                                /// TODO: Forward the TC to the COMMS Execution Task
                                LOG_DEBUG << "Received TC from UART destined for COMMS";
                            }
                        }
                    }
                    else {
                        LOG_ERROR << "TC_BUF NULL POINTER";
                    }
                }else {
                    LOG_ERROR << "TIMEOUT";
                }
            }
            else {
                LOG_ERROR << "OTHER EVENT";
            }
        }
    }
}