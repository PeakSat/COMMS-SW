#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "GNSSTask.hpp"


void app_main(void) {
    //    transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    //    eMMCTask.emplace();
    gnssTask.emplace();


    //    transceiverTask->createTask();
    uartGatekeeperTask->createTask();
    //    eMMCTask->createTask();
    gnssTask->createTask();

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
extern "C" void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
    // Declare a variable to track if a higher priority task is woken up
    BaseType_t xHigherPriorityTaskWoken;
    // Initialize xHigherPriorityTaskWoken to pdFALSE (no higher-priority task woken yet)
    xHigherPriorityTaskWoken = pdFALSE;
    if (huart->Instance == UART5) {
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE) {
            gnssTask->size = Size;
            // if xTaskNotifyFromISR() sets the value of xHigherPriorityTaskWoken TO pdTRUE then a context switch should be requested before the interrupt is exited.
            if (gnssTask->control) {
                xTaskNotifyFromISR(gnssTask->taskHandle, GNSS_RESPONSE, eSetBits, &xHigherPriorityTaskWoken);
                gnssTask->sendToQueueResponse = huart5.pRxBuffPtr;
                xQueueSendFromISR(gnssTask->gnssQueueHandleGNSSResponse, &gnssTask->sendToQueueResponse, &xHigherPriorityTaskWoken);
            } else {
                xTaskNotifyFromISR(gnssTask->taskHandle, GNSS_MESSAGE_READY, eSetBits, &xHigherPriorityTaskWoken);
                gnssTask->sendToQueue = huart5.pRxBuffPtr;
                xQueueSendFromISR(gnssTask->gnssQueueHandleDefault, &gnssTask->sendToQueue, &xHigherPriorityTaskWoken);
            }
            // Perform a context switch if a higher-priority task was woken up by the notification
            // portYIELD_FROM_ISR will yield the processor to the higher-priority task immediately if xHigherPriorityTaskWoken is pdTRUE
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            // restart the DMA //
            gnssTask->startReceiveFromUARTwithIdle(gnssTask->rx_buf_pointer, 1024);
            //
        }
    }
}
