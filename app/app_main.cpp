#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"
#include "INA3221Task.hpp"
#include "TMP117Task.hpp"
#include "eMMC.hpp"


void app_main(void) {

    eMMC::eMMCMemoryInit();
    if (eMMC::memoryMap[eMMC::firmware].endAddress != 0) {
        __NOP();
    }

    transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    eMMCTask.emplace();
    ina3221Task.emplace();
    tmp117Task.emplace();

    transceiverTask->createTask();
    uartGatekeeperTask->createTask();
    eMMCTask->createTask();
    ina3221Task->createTask();
    tmp117Task->createTask();

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