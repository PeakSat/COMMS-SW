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

void TCHandlingTask::startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size) {
    // start the DMA
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, buf, size);
    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
    //  disabling the full buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_TC);
}


[[noreturn]] void TCHandlingTask::execute() {
    tc_buf_dma_pointer = TC_BUF;
    startReceiveFromUARTwithIdle(tc_buf_dma_pointer, 512);
    uint32_t received_events;
    TCUARTQueueHandle = xQueueCreateStatic(TCUARTQueueSize, sizeof(uint8_t*), incomingTCUARTQueueStorageArea,
                                        &incomingTCQueueBuffer);
    vQueueAddToRegistry(TCUARTQueueHandle, "TC UART queue");

    LOG_INFO << "TCHandlingTask::execute()";
    CAN::StoredPacket TC_PACKET;
    uint8_t* tc_buf_from_queue_pointer;
    uint8_t ECSS_TC_BUF[1024];
    uint8_t TC_BUFFER[1024];
    uint32_t eMMCPacketTailPointer = 0;
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
                if (xQueueReceive(TCUARTQueueHandle, &tc_buf_from_queue_pointer, pdMS_TO_TICKS(100)) == pdTRUE) {
                    if (tc_buf_from_queue_pointer != nullptr) {
                        LOG_DEBUG << "RECEIVED TC FROM UART-YAMCS, size: " << size;
                        for (uint16_t i = 0; i < size; i++) {
                            LOG_DEBUG << "Received TC data: " << tc_buf_from_queue_pointer[i];
                            ECSS_TC_BUF[i] = tc_buf_from_queue_pointer[i];
                        }
                        if (ECSS_TC_BUF[2] == OBC_APPLICATION_ID) {
                            auto status = storeItem(eMMC::memoryMap[eMMC::UART_TC], ECSS_TC_BUF, 1024, eMMCPacketTailPointer, 2);
                            if (status.has_value()) {
                                CAN::StoredPacket PacketToBeStored;
                                PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
                                eMMCPacketTailPointer += 2;
                                PacketToBeStored.size = size;
                                xQueueSendToBack(TXQueue, &PacketToBeStored, 0);
                                xTaskNotifyIndexed(rf_txtask->taskHandle, NOTIFY_INDEX_TRANSMIT, TC_UART_TC_HANDLING_TASK, eSetBits);
                            }
                            else {
                                LOG_ERROR << "MEMORY ERROR ON TC";
                            }
                        }
                        else if (ECSS_TC_BUF[2] == COMMS_APPLICATION_ID) {
                            /// TODO: Forward the TC to the COMMS Execution Task
                            LOG_DEBUG << "RECEIVED COMMS_APPLICATION_ID";
                        }
                    }
                }
            }
        }
    }
}