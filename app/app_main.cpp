#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"

struct eMMCTransactionHandler eMMCTransactionHandler;

uint32_t memoryCapacity = 0xE90E80;
uint32_t memoryPageSize = 512; // bytes

enum memoryItem {
    firmware,
    GNSSData,
    memoryItemCount
};

struct memoryItemData {
    memoryItemData() : size(0), startAddress(), endAddress() {}
    explicit memoryItemData(uint32_t newSize)
        : size(newSize), startAddress(0), endAddress(0) {}
    uint32_t size;         // Item size in bytes
    uint32_t startAddress; // Calculated automatically on boot
    uint32_t endAddress;   // Calculated automatically on boot
};

void app_main(void) {

    std::array<memoryItemData, memoryItemCount> memoryMap;

    memoryMap[firmware] = memoryItemData(1000000);
    memoryMap[GNSSData] = memoryItemData(3000000);

    uint32_t headPointer = 0;
    for (int i = 0; i < memoryItemCount; i++) {
        headPointer += memoryMap[i].size;
        if (headPointer > memoryCapacity) {
            // memory full
        } else {
            memoryMap[i].endAddress = headPointer;
            memoryMap[i].startAddress = headPointer - memoryMap[i].size;
        }

        // allign for page size (512 bytes)
        headPointer += memoryPageSize - (headPointer % memoryPageSize);
    }

    if (memoryMap[0].endAddress != 0) {
        __NOP();
    }

    eMMCTransactionHandler.eMMC_semaphore = xSemaphoreCreateMutex();

    transceiverTask.emplace();
    uartGatekeeperTask.emplace();
    eMMCTask.emplace();

    transceiverTask->createTask();
    uartGatekeeperTask->createTask();
    eMMCTask->createTask();

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