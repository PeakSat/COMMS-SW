#pragma once
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "GNSSTask.hpp"
#include "TestTask.hpp"
#include "INA3221Task.hpp"
#include "CANGatekeeperTask.hpp"
#include "TPProtocol.hpp"
#include "CANParserTask.hpp"
#include <optional>
#include "TMP117Task.hpp"
#include "CANDriver.hpp"
#include "eMMC.hpp"
#include "RF_TXTask.hpp"
#include "RF_RXTask.hpp"
#include "git_version.h"
#include <ServicePool.hpp>
#include "at86rf215.hpp"
#include "HeartbeatTask.hpp"
#include "TCHandlingTask.hpp"
#include "TMHandlingTask.hpp"

#define ENABLE_UART4_RX

#include <stm32h7xx_it.h>

ParameterService parameterMap;

void app_main(void) {

    eMMC::eMMCMemoryInit();
    if (eMMC::memoryMap[eMMC::firmware].endAddress != 0) {
        __NOP();
    }
    transceiver.setGeneralConfig(GeneralConfiguration::DefaultGeneralConfig());
    transceiver.setRXConfig(RXConfig::DefaultRXConfig());
    transceiver.setTXConfig(TXConfig::DefaultTXConfig());
    transceiver.setBaseBandCoreConfig(BasebandCoreConfig::DefaultBasebandCoreConfig());
    transceiver.setFrequencySynthesizerConfig(FrequencySynthesizer::DefaultFrequencySynthesizerConfig());
    transceiver.setExternalFrontEndControlConfig(ExternalFrontEndConfig::DefaultExternalFrontEndConfig());
    transceiver.setInterruptConfig(InterruptsConfig::DefaultInterruptsConfig());
    transceiver.setRadioInterruptConfig(RadioInterruptsConfig::DefaultRadioInterruptsConfig());
    transceiver.setIQInterfaceConfig(IQInterfaceConfig::DefaultIQInterfaceConfig());

    uartGatekeeperTask.emplace();
    rf_rxtask.emplace();
    rf_txtask.emplace();
    eMMCTask.emplace();
    gnssTask.emplace();
    testTask.emplace();
    ina3221Task.emplace();
    canGatekeeperTask.emplace();
    tmp117Task.emplace();
    canParserTask.emplace();
    heartbeatTask.emplace();
    tcHandlingTask.emplace();
    tmhandlingTask.emplace();
    heartbeatTask.emplace();

    uartGatekeeperTask->createTask();
    // rf_rxtask->createTask();
    // rf_txtask->createTask();
    eMMCTask->createTask();
    gnssTask->createTask();
    testTask->createTask();
    ina3221Task->createTask();
    canGatekeeperTask->createTask();
    tmp117Task->createTask();
    canParserTask->createTask();
    tcHandlingTask->createTask();
    tmhandlingTask->createTask();
    heartbeatTask->createTask();
    // HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    LOG_INFO << "####### This board runs COMMS_Software, commit " << kGitHash << " #######";
    COMMSParameters::COMMIT_HASH.setValue(static_cast<uint32_t>(std::stoul(kGitHash, nullptr, 16)));
    LOG_INFO << "eMMC usage = " << COMMSParameters::EMMC_USAGE.getValue() << "%";
    /* Start the scheduler. */
    can_ack_handler.initialize_semaphore();
    CAN_TRANSMIT_Handler.initialize_semaphore();
    transceiver_handler.initialize_semaphore();
    HAL_NVIC_DisableIRQ(DMA1_Stream2_IRQn);
#ifdef ENABLE_UART4_RX
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
#endif

    vTaskStartScheduler();

    /* Should not get here. */
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

extern "C" [[maybe_unused]] void EXTI1_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(RF_IRQ_Pin);
    transceiver.handle_irq();
}

/* Callback in non blocking modes (DMA) */
extern "C" [[maybe_unused]] void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef* hmmc) {
    eMMC::eMMCTransactionHandler.WriteComplete = true;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(eMMC::eMMCTransactionHandler.eMMC_writeCompleteSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // __NOP();
}
extern "C" [[maybe_unused]] void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef* hmmc) {
    eMMC::eMMCTransactionHandler.ReadComplete = true;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(eMMC::eMMCTransactionHandler.eMMC_readCompleteSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {

        while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0) {
            /* Retrieve Rx messages from RX FIFO0 */
            CAN::Frame newFrame;
            if (incomingFIFO.lastItemPointer >= sizeOfIncommingFrameBuffer) {
                incomingFIFO.lastItemPointer = 0;
            }
            newFrame.pointerToData = &incomingFIFO.buffer[CANMessageSize * (incomingFIFO.lastItemPointer)];
            if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &newFrame.header, newFrame.pointerToData) != HAL_OK) {
                /* Reception Error */
                Error_Handler();
            }
            newFrame.bus = hfdcan;

            if (newFrame.pointerToData[0] == 0 && newFrame.pointerToData[1] == 0) {
                __NOP();
                if (newFrame.bus->Instance == FDCAN1) {
                    __NOP();
                } else if (newFrame.bus->Instance == FDCAN2) {
                    __NOP();
                }
            } else if (newFrame.header.Identifier == 0x380) {
                if (xQueueIsQueueFullFromISR(canGatekeeperTask->incomingFrameQueue)) {
                    // Queue is full. Handle the error
                    // todo
                    __NOP();
                } else {
                    // Send the data to the gatekeeper
                    incomingFIFO.lastItemPointer++;
                    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                    xQueueSendToBackFromISR(canGatekeeperTask->incomingFrameQueue, &newFrame, NULL);
                    xTaskNotifyFromISR(canGatekeeperTask->taskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
                    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    __NOP();
                }
            } else {
                __NOP();
            }
        }
    }
    // Re-activate the callback
    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        /* Notification Error */
        Error_Handler();
    }
}
extern "C" void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
    // Declare a variable to track if a higher priority task is woken up
    BaseType_t xHigherPriorityTaskWoken;
    // Initialize xHigherPriorityTaskWoken to pdFALSE (no higher-priority task woken yet)
    xHigherPriorityTaskWoken = pdFALSE;

    // UART5 -> GNSS
    if (huart->Instance == UART5) {
        auto currentsGNSSBufferTailPointer = static_cast<uint32_t>(Size);

        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE) {
            uint32_t currentSize = currentsGNSSBufferTailPointer - previousGNSSBufferTailPointer;
            // move data to the beginning of the buffer
            if (currentsGNSSBufferTailPointer > previousGNSSBufferTailPointer) {
                if (previousGNSSBufferTailPointer > 1024) {
                    __NOP();
                }
                for (int i = 0; i < currentSize; i++) {
                    huart5.pRxBuffPtr[i] = huart5.pRxBuffPtr[i + previousGNSSBufferTailPointer];
                }
            } else {
                currentSize = huart5.RxXferSize - previousGNSSBufferTailPointer;
                currentSize += currentsGNSSBufferTailPointer;
                for (int i = static_cast<int>(currentsGNSSBufferTailPointer); i > 0; i--) {
                    huart5.pRxBuffPtr[static_cast<uint32_t>(i) + (huart5.RxXferSize - previousGNSSBufferTailPointer)] = huart5.pRxBuffPtr[i];
                }
                for (uint32_t i = previousGNSSBufferTailPointer; i < huart5.RxXferSize; i++) {
                    huart5.pRxBuffPtr[i - previousGNSSBufferTailPointer] = huart5.pRxBuffPtr[i];
                }
            }
            // Size = 9 have the messages with main ID and Size = 10 have the messages with Sub ID
            if (gnssTask->gnss_handler.CONTROL && (currentSize == gnssTask->gnss_handler.SIZE_ID_LENGTH || currentSize == gnssTask->gnss_handler.SIZE_SUB_ID_LENGTH)) {
                gnssTask->gnss_handler.CONTROL = false;
                if (huart5.pRxBuffPtr[4] == gnssTask->gnss_handler.ACK)
                    xTaskNotifyIndexedFromISR(gnssTask->taskHandle, GNSS_INDEX_ACK, GNSS_ACK, eSetBits, &xHigherPriorityTaskWoken);
            } else {
                gnssTask->size_message = currentSize;
                gnssTask->sendToQueue = huart5.pRxBuffPtr;
                xHigherPriorityTaskWoken = pdFALSE;
                xTaskNotifyIndexedFromISR(gnssTask->taskHandle, GNSS_INDEX_MESSAGE, GNSS_MESSAGE_READY, eSetBits, &xHigherPriorityTaskWoken);
                xQueueSendFromISR(gnssTask->gnssQueueHandle, &gnssTask->sendToQueue, &xHigherPriorityTaskWoken);
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            previousGNSSBufferTailPointer = static_cast<uint32_t>(currentsGNSSBufferTailPointer);
        }
    }

    // UART4 -> Logger
    if (huart->Instance == UART4) {
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE) {
            if (Size >= 5 && Size <= 1024) {
                tcHandlingTask->tc_uart_var = true;
                tcHandlingTask->send_to_tc_queue = huart4.pRxBuffPtr;
                tcHandlingTask->size = Size;
                xHigherPriorityTaskWoken = pdFALSE;
                xQueueSendFromISR(tcHandlingTask->TCUARTQueueHandle, &tcHandlingTask->send_to_tc_queue, &xHigherPriorityTaskWoken);
                xHigherPriorityTaskWoken = pdFALSE;
                xTaskNotifyIndexedFromISR(tcHandlingTask->taskHandle, NOTIFY_INDEX_INCOMING_TC, (1 << 18), eSetBits, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            TCHandlingTask::startReceiveFromUARTwithIdle(tcHandlingTask->tc_buf_dma_pointer, 1024);
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    BaseType_t xHigherPriorityTaskWoken = false;
    if (huart->Instance == UART4) {
        xSemaphoreGiveFromISR(UART_Gatekeeper_Semaphore, &xHigherPriorityTaskWoken);
    }
}