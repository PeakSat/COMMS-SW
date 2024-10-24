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
            // this flag triggers the GNSS task to start the processing of GNSS data //
            gnssTask->gnss_flag = true;
            // restart the DMA //
            HAL_UARTEx_ReceiveToIdle_DMA(&huart5, gnssTask->incomingMessage, 512);
            // disabling the half buffer interrupt //
            __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
            // disabling the full buffer interrupt //
            __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);
        }
    }
}
