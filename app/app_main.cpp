#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "GNSSTask.hpp"

uint8_t receivedByte = 0;
bool controlGNSSenabled = false;
bool ack_flag = false;
bool nack_flag = false;
bool uart_flag_it = false;


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
            if (controlGNSSenabled) {
                // Declare a variable to track if a higher priority task is woken up
                BaseType_t xHigherPriorityTaskWokenForControl;
                // Initialize xHigherPriorityTaskWoken to pdFALSE (no higher-priority task woken yet)
                xHigherPriorityTaskWokenForControl = pdFALSE;
                for (uint16_t i = 0; i < Size; i++) {
                    if (gnssTask->incomingMessage[i] == 131) {
                        // ACK
                        xTaskNotifyFromISR(gnssTask->taskHandle, 1, eNoAction, &xHigherPriorityTaskWokenForControl);
                        portYIELD_FROM_ISR(xHigherPriorityTaskWokenForControl);

                        HAL_UARTEx_ReceiveToIdle_DMA(&huart5, gnssTask->incomingMessage, 512);
                        // disabling the half buffer interrupt //
                        __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
                        // disabling the full buffer interrupt //
                        __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);
                    } else if (gnssTask->incomingMessage[i] == 132) {
                        // NACK
                        nack_flag = true;
                        xTaskNotifyFromISR(gnssTask->taskHandle, 2, eNoAction, &xHigherPriorityTaskWokenForControl);
                        portYIELD_FROM_ISR(xHigherPriorityTaskWokenForControl);

                        HAL_UARTEx_ReceiveToIdle_DMA(&huart5, gnssTask->incomingMessage, 512);
                        // disabling the half buffer interrupt //
                        __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
                        // disabling the full buffer interrupt //
                        __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);
                    }
                }
                controlGNSSenabled = false;
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

//extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//
//    if (receivedByte == 131){
//        // ACK
//        __NOP();
//        ack_flag = true;
//    } else if (receivedByte == 132) {
//        // NACK
//        __NOP();
//        nack_flag = true;
//        }
//        HAL_UART_Receive_IT(&huart5, &receivedByte, 1);  // This will trigger an interrupt on receiving a byte
//}


//extern "C" void USART5_IRQHandler(void) {
//    uart_flag_it = true;
//        receivedByte = huart5.Instance->RDR;
//        {
//            if (receivedByte == 131) {
//                  // ACK received
//                  ack_flag = true;
//                  return;
//            } else if (receivedByte == 131) {
//                nack_flag = true;
//                return;
//            }
//        }
//        // You can also set up the next interrupt if needed
//        HAL_UART_Receive_IT(&huart5, &receivedByte, 1); // Re-enable the interrupt for the next byte
//
//}
