
#include "eMMC.hpp"
#include "app_main.h"

using namespace eMMC;
extern MMC_HandleTypeDef hmmc1;

/**
 * 
 * @param write_data 
 * @param block_address 
 * @param numberOfBlocks
 * @return 
 */
etl::expected<void, Error> eMMC::writeBlockEMMC(uint8_t* write_data, uint32_t block_address, uint32_t numberOfBlocks) {

    xSemaphoreTake(eMMCTransactionHandler.eMMC_semaphore, portMAX_DELAY);
    eMMCTransactionHandler.WriteComplete = false;
    eMMCTransactionHandler.ErrorOccured = false;
    eMMCTransactionHandler.transactionAborted = false;

    HAL_StatusTypeDef status = HAL_MMC_WriteBlocks_IT(&hmmc1, write_data, block_address, numberOfBlocks);
    if (status != HAL_OK) {

        xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
        return etl::unexpected<Error>(Error::EMMC_WRITE_FAILURE);
    }

    uint32_t startTime = xTaskGetTickCount();
    while (true) {
        vTaskDelay(1);
        if (eMMCTransactionHandler.WriteComplete == true) {
            xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
            return {};
        }

        // Transaction timed out
        if (xTaskGetTickCount() > ((eMMCTransactionHandler.transactionTimeoutPerBlock * numberOfBlocks) + startTime)) {
            xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
            return etl::unexpected<Error>(Error::EMMC_TRANSACTION_TIMED_OUT);
        }

        // Error calback was called
        if (eMMCTransactionHandler.ErrorOccured == true) {
            // Handle the error. Check eMMCTransactionHandler.hmmcSnapshot for error messages.

            xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
            return etl::unexpected<Error>(Error::EMMC_WRITE_FAILURE);
        }
    }
}

/**
 * 
 * @param read_data 
 * @param block_address 
 * @param numberOfBlocks
 * @return 
 */
etl::expected<void, Error> eMMC::readBlockEMMC(uint8_t* read_data, uint32_t block_address, uint32_t numberOfBlocks) {

    xSemaphoreTake(eMMCTransactionHandler.eMMC_semaphore, portMAX_DELAY);
    eMMCTransactionHandler.ReadComplete = false;
    eMMCTransactionHandler.ErrorOccured = false;
    eMMCTransactionHandler.transactionAborted = false;

    HAL_StatusTypeDef status = HAL_MMC_ReadBlocks_IT(&hmmc1, read_data, block_address, numberOfBlocks);
    if (status != HAL_OK) {

        xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
        return etl::unexpected<Error>(Error::EMMC_READ_FAILURE);
    }

    uint32_t startTime = xTaskGetTickCount();
    while (true) {
        vTaskDelay(1);
        if (eMMCTransactionHandler.ReadComplete == true) {
            xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
            return {};
        }

        // Transaction timed out
        if (xTaskGetTickCount() > ((eMMCTransactionHandler.transactionTimeoutPerBlock * numberOfBlocks) + startTime)) {
            xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
            return etl::unexpected<Error>(Error::EMMC_TRANSACTION_TIMED_OUT);
        }

        // Error calback was called
        if (eMMCTransactionHandler.ErrorOccured == true) {
            // Handle the error. Check eMMCTransactionHandler.hmmcSnapshot for error messages.

            xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
            return etl::unexpected<Error>(Error::EMMC_READ_FAILURE);
        }
    }
}

/**
 * 
 * @param block_address_start 
 * @param block_address_end 
 * @return 
 */
etl::expected<void, Error> eMMC::eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end) {
    if (block_address_start > block_address_end)
        return etl::unexpected<Error>(Error::EMMC_INVALID_START_ADDRESS_ON_ERASE);
    if (HAL_MMC_Erase(&hmmc1, block_address_start, block_address_end) != HAL_OK)
        return etl::unexpected<Error>(Error::EMMC_ERASE_BLOCK_FAILURE);
    return {}; // success
}
