#include "TCHandlingTask.hpp"
#include "RF_RXTask.hpp"
#include "Logger.hpp"

#include <ApplicationLayer.hpp>
#include <COBS.hpp>
#include <MessageParser.hpp>
#include <at86rf215definitions.hpp>
#include <eMMC.hpp>
#include "Message.hpp"

void TCHandlingTask::startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size) {
    // start the DMA
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, buf, size);
    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
    //  disabling the full buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_TC);
}

[[noreturn]] void TCHandlingTask::execute() {
    LOG_INFO << "TCHandlingTask::execute()";
    tc_buf_dma_pointer = TC_BUF;
    startReceiveFromUARTwithIdle(tc_buf_dma_pointer, 512);
    uint32_t received_events;
    TCQueueHandle = xQueueCreateStatic(TCQueueSize, sizeof(uint8_t*), incomingTCQueueStorageArea,
                                        &incomingTCQueueBuffer);
    vQueueAddToRegistry( TCQueueHandle, "TC queue");

    uint8_t* tc_buf_from_queue_pointer;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RECEIVED_TC, pdFALSE, pdTRUE, &received_events, portMAX_DELAY) == pdTRUE) {
            if (xQueueReceive(TCQueueHandle, &tc_buf_from_queue_pointer, pdMS_TO_TICKS(100)) == pdTRUE) {
                // TODO parse the TC
                __NOP();
                if (tc_buf_from_queue_pointer != nullptr)
                    LOG_DEBUG << "RECEIVED TC FROM UART, size: " << size;
                    for (uint16_t i = 0; i < size; i++) {
                        LOG_DEBUG << "Received TC data: " << tc_buf_from_queue_pointer[i];
                    }
            }
        }
    }
}