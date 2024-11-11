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
    if (gnssTask->buffer_var) {
        gnssTask->active_buffer_pointer = const_cast<uint8_t*>(gnssTask->first_buffer);
        gnssTask->buffer_var = 0;
    } else {
        gnssTask->active_buffer_pointer = const_cast<uint8_t*>(gnssTask->second_buffer);
        gnssTask->buffer_var = 1;
    }
    if (huart->Instance == UART5) {
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE) {

            gnssTask->size = Size;
            if (Size <= MAX_EXPECTED_GNSS_RESPONSE) {
                xTaskNotifyFromISR(gnssTask->taskHandle, GNSS_RESPONSE, eSetBits, &xHigherPriorityTaskWoken);
            } else if (Size == EXPECTED_GNSS_MESSAGE) {
                // if xTaskNotifyFromISR() sets the value of xHigherPriorityTaskWoken TO pdTRUE then a context switch should be requested before the interrupt is exited.
                xTaskNotifyFromISR(gnssTask->taskHandle, GNSS_MESSAGE_READY, eSetBits, &xHigherPriorityTaskWoken);
            }
            // Perform a context switch if a higher-priority task was woken up by the notification
            // portYIELD_FROM_ISR will yield the processor to the higher-priority task immediately if xHigherPriorityTaskWoken is pdTRUE
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            // restart the DMA //

            HAL_UARTEx_ReceiveToIdle_DMA(&huart5, const_cast<uint8_t*>(gnssTask->active_buffer_pointer), 512);

            // disabling the half buffer interrupt //
            __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
            // disabling the full buffer interrupt //
            __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);
        }
    }
}
