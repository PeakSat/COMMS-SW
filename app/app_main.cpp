#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "INA3221Task.hpp"

struct eMMCTransactionHandler eMMCTransactionHandler;

void app_main(void) {
    eMMCTransactionHandler.eMMC_semaphore = xSemaphoreCreateMutex();

    transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    eMMCTask.emplace();
    ina3221Task.emplace();

    transceiverTask->createTask();
    uartGatekeeperTask->createTask();
    eMMCTask->createTask();
    ina3221Task->createTask();

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