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
#include "CANTestTask.hpp"
#include <optional>
#include "TMP117Task.hpp"
#include "CANDriver.hpp"
#include "eMMC.hpp"


void app_main(void) {

    eMMC::eMMCMemoryInit();
    if (eMMC::memoryMap[eMMC::firmware].endAddress != 0) {
        __NOP();
    }

    // transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    eMMCTask.emplace();
    ina3221Task.emplace();
    canGatekeeperTask.emplace();
    tmp117Task.emplace();
    canTestTask.emplace();

    // transceiverTask->createTask();
    uartGatekeeperTask->createTask();
    eMMCTask->createTask();
    ina3221Task->createTask();
    canGatekeeperTask->createTask();
    tmp117Task->createTask();
    canTestTask->createTask();

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

/**
  * probably wont need this
  */
extern "C" void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo1ITs) {
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    //
    // __NOP();
    //
    //
    //
    // if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) {
    //     /* Retreive Rx messages from RX FIFO1 */
    //     if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &CAN::rxHeader1, CAN::rxFifo1.data()) != HAL_OK) {
    //         /* Reception Error */
    //         Error_Handler();
    //     }
    //     //        canGatekeeperTask->switchActiveBus(CAN::Redundant);
    //     CAN::rxFifo1.repair();
    //     CAN::Packet newFrame = CAN::getFrame(&CAN::rxFifo1, CAN::rxHeader1.Identifier);
    //     if (CAN::rxFifo0[0] >> 6 == CAN::TPProtocol::Frame::Single) {
    //         canGatekeeperTask->addSFToIncoming(newFrame);
    //         xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
    //
    //     } else {
    //         canGatekeeperTask->addMFToIncoming(newFrame);
    //         if (CAN::rxFifo0[0] >> 6 == CAN::TPProtocol::Frame::Final) {
    //             xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
    //             portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    //         }
    //     }
    //
    //     if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK) {
    //         /* Notification Error */
    //         Error_Handler();
    //     }
    // }
}

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    __NOP();
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
        /* Retrieve Rx messages from RX FIFO0 */

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN::rxHeader0, &incomingFIFO.buffer[64 * (incomingFIFO.lastItemPointer)]) != HAL_OK) {
            /* Reception Error */
            Error_Handler();
        }

        CAN::rxFifo0.repair();
        CAN::Packet newPacket = CAN::getFrame(&CAN::rxFifo0, CAN::rxHeader0.Identifier);
        newPacket.dataPointer = &incomingFIFO.buffer[64 * (incomingFIFO.lastItemPointer)];
        incomingFIFO.lastItemPointer++;

        if (incomingFIFO.lastItemPointer >= incomingFIFO.NOfItems) {
            incomingFIFO.lastItemPointer = 0;
        }
        newPacket.bus = hfdcan;

        // if(hfdcan->Instance == FDCAN1) {
        //     __NOP();
        //
        // }else if(hfdcan->Instance == FDCAN2){
        //     __NOP();
        //
        // }

        canGatekeeperTask->addSFToIncoming(newPacket);
        xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);


        // if(CAN::rxFifo0[0] >> 6 == CAN::TPProtocol::Frame::Single){
        //     canGatekeeperTask->addSFToIncoming(newFrame);
        //     xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
        //
        // }else{
        //     canGatekeeperTask->addMFToIncoming(newFrame);
        //     if(CAN::rxFifo0[0] >> 6 == CAN::TPProtocol::Frame::Final){
        //         xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
        //         portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        //     }
        // }

        if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
            /* Notification Error */
            Error_Handler();
        }
    }
}