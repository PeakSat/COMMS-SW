#pragma once
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "INA3221Task.hpp"
#include "CANGatekeeperTask.hpp"
#include "TPProtocol.hpp"


struct eMMCTransactionHandler eMMCTransactionHandler;

void app_main(void) {
    eMMCTransactionHandler.eMMC_semaphore = xSemaphoreCreateMutex();

    transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    eMMCTask.emplace();
    ina3221Task.emplace();
    canGatekeeperTask.emplace();

    transceiverTask->createTask();
    uartGatekeeperTask->createTask();
    eMMCTask->createTask();
    ina3221Task->createTask();
    canGatekeeperTask->createTask();

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
    struct eMMCTransactionHandler* test = &eMMCTransactionHandler;
    test->WriteComplete = true;
    // __NOP();
}
extern "C" [[maybe_unused]] void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef* hmmc) {
    eMMCTransactionHandler.ReadComplete = true;
    // __NOP();
}
extern "C" [[maybe_unused]] void HAL_MMC_ErrorCallback(MMC_HandleTypeDef* hmmc) {
    eMMCTransactionHandler.ErrorOccured = true;
    eMMCTransactionHandler.hmmcSnapshot = *hmmc;
    // __NOP();
}
extern "C" [[maybe_unused]] void HAL_MMC_AbortCallback(MMC_HandleTypeDef* hmmc) {
    eMMCTransactionHandler.transactionAborted = true;
    // __NOP();
}


extern "C" void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo1ITs) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) {
        /* Retreive Rx messages from RX FIFO1 */
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &CAN::rxHeader1, CAN::rxFifo1.data()) != HAL_OK) {
            /* Reception Error */
            Error_Handler();
        }
        //        canGatekeeperTask->switchActiveBus(CAN::Redundant);
        CAN::rxFifo1.repair();
        CAN::Frame newFrame = CAN::getFrame(&CAN::rxFifo1, CAN::rxHeader1.Identifier);
        if (CAN::rxFifo0[0] >> 6 == CAN::TPProtocol::Frame::Single) {
            canGatekeeperTask->addSFToIncoming(newFrame);
            xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);

        } else {
            canGatekeeperTask->addMFToIncoming(newFrame);
            if (CAN::rxFifo0[0] >> 6 == CAN::TPProtocol::Frame::Final) {
                xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }

        if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK) {
            /* Notification Error */
            Error_Handler();
        }
    }
}
