#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "GNSSTask.hpp"

bool ack_flag = false;
bool nack_flag = false;

void app_main(void) {
    transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    //    eMMCTask.emplace();
    gnssTask.emplace();


    transceiverTask->createTask();
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
    if (huart->Instance == UART5) {
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE) {
            gnssTask->size = Size;
            if (Size > 0 && Size < 460) {
                // 460 bytes is the nominal size for the message, so everything
                // smaller than that is response to a configuration message
                // usually it under 20 bytes
                for (uint16_t i = 0; i < Size; i++) {
                    if (gnssTask->incomingMessage[i] == 131) {
                        // ACK
                        ack_flag = true;
                    }
                    if (gnssTask->incomingMessage[i] == 132) {
                        // NACK
                        nack_flag = true;
                    }
                }
            }
            // Declare a variable to track if a higher priority task is woken up
            BaseType_t xHigherPriorityTaskWoken;
            // Initialize xHigherPriorityTaskWoken to pdFALSE (no higher-priority task woken yet)
            xHigherPriorityTaskWoken = pdFALSE;
            // notify the gnssTask
            // if xTaskNotifyFromISR() sets the value of xHigherPriorityTaskWoken TO pdTRUE then a context switch should be requested before the interrupt is exited.
            xTaskNotifyFromISR(gnssTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
            // Perform a context switch if a higher-priority task was woken up by the notification
            // portYIELD_FROM_ISR will yield the processor to the higher-priority task immediately if xHigherPriorityTaskWoken is pdTRUE
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            // restart the DMA //
            HAL_UARTEx_ReceiveToIdle_DMA(&huart5, gnssTask->incomingMessage, 512);
            // disabling the half buffer interrupt //
            __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
            // disabling the full buffer interrupt //
            __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);
        }
    }
}
