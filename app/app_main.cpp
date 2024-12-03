#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "GNSSTask.hpp"

#include "INA3221Task.hpp"
#include "TMP117Task.hpp"
#include "eMMC.hpp"


void app_main(void) {

    eMMC::eMMCMemoryInit();
    if (eMMC::memoryMap[eMMC::firmware].endAddress != 0) {
        __NOP();
    }

    //    transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    //    eMMCTask.emplace();
    gnssTask.emplace();

    //    ina3221Task.emplace();
    //    tmp117Task.emplace();

    //    transceiverTask->createTask();
    uartGatekeeperTask->createTask();
    //    eMMCTask->createTask();
    gnssTask->createTask();
    //    ina3221Task->createTask();
    //    tmp117Task->createTask();

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Should not get here. */
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

extern "C" [[maybe_unused]] void EXTI1_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(RF_IRQ_Pin);
    transceiverTask->transceiver.handle_irq();
}

/* Callback in non blocking modes (DMA) */
extern "C" [[maybe_unused]] void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef* hmmc) {
    struct eMMC::eMMCTransactionHandler* test = &eMMC::eMMCTransactionHandler;
    test->WriteComplete = true;
    // __NOP();
}
extern "C" [[maybe_unused]] void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef* hmmc) {
    eMMC::eMMCTransactionHandler.ReadComplete = true;
    // __NOP();
}
extern "C" [[maybe_unused]] void HAL_MMC_ErrorCallback(MMC_HandleTypeDef* hmmc) {
    eMMC::eMMCTransactionHandler.ErrorOccured = true;
    eMMC::eMMCTransactionHandler.hmmcSnapshot = *hmmc;
    // __NOP();
}
extern "C" [[maybe_unused]] void HAL_MMC_AbortCallback(MMC_HandleTypeDef* hmmc) {
    eMMC::eMMCTransactionHandler.transactionAborted = true;
    // __NOP();
}
extern "C" void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
    // Declare a variable to track if a higher priority task is woken up
    BaseType_t xHigherPriorityTaskWoken;
    // Initialize xHigherPriorityTaskWoken to pdFALSE (no higher-priority task woken yet)
    xHigherPriorityTaskWoken = pdFALSE;
    if (huart->Instance == UART5) {
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE) {
            // if xTaskNotifyFromISR() sets the value of xHigherPriorityTaskWoken TO pdTRUE then a context switch should be requested before the interrupt is exited.
            // Size = 9 have the messages with main ID and Size = 10 have the messages with Sub ID
            if (gnssTask->control && (Size == 9 || Size == 10)) {
                gnssTask->size_response = Size;
                gnssTask->sendToQueueResponse = huart5.pRxBuffPtr;
                xQueueSendFromISR(gnssTask->gnssQueueHandleGNSSResponse, &gnssTask->sendToQueueResponse, &xHigherPriorityTaskWoken);
                xTaskNotifyFromISR(gnssTask->taskHandle, GNSS_RESPONSE, eSetBits, &xHigherPriorityTaskWoken);
            } else {
                gnssTask->size_message = Size;
                gnssTask->sendToQueue = huart5.pRxBuffPtr;
                xQueueSendFromISR(gnssTask->gnssQueueHandleDefault, &gnssTask->sendToQueue, &xHigherPriorityTaskWoken);
                xTaskNotifyFromISR(gnssTask->taskHandle, GNSS_MESSAGE_READY, eSetBits, &xHigherPriorityTaskWoken);
            }
            // Perform a context switch if a higher-priority task was woken up by the notification
            // portYIELD_FROM_ISR will yield the processor to the higher-priority task immediately if xHigherPriorityTaskWoken is pdTRUE
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            // restart the DMA //
            gnssTask->startReceiveFromUARTwithIdle(gnssTask->rx_buf_pointer, 1024);
        }
    }
}
