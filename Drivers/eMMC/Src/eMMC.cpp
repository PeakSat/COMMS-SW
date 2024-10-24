
#include "eMMC.hpp"
#include "app_main.h"

using namespace eMMC;
extern MMC_HandleTypeDef hmmc1;

namespace eMMC {
    // Define the global variables
    uint32_t memoryCapacity = 0xE90E80;
    uint32_t memoryPageSize = 512; // bytes
    std::array<memoryItemHandler, memoryItemCount> memoryMap;
} // namespace eMMC


/**
 *
 */
void eMMC::eMMCMemoryInit() {

    // Initialize the memoryMap array using the sizes from MemoryItems.def
#define MEMORY_ITEM(name, size) memoryMap[name] = memoryItemHandler(size);
#include "MemoryItems.def"
#undef MEMORY_ITEM

    if (memoryMap[0].endAddress != 0) {
        __NOP();
    }
    uint32_t headPointer = 0;
    for (int i = 0; i < memoryItemCount; i++) {
        headPointer += memoryMap[i].size;
        if (headPointer > memoryCapacity) {
            // memory full
        } else {
            memoryMap[i].endAddress = headPointer;
            memoryMap[i].startAddress = headPointer - memoryMap[i].size;
        }

        // align for page size (512 bytes)
        headPointer += memoryPageSize - (headPointer % memoryPageSize);
    }
    if (memoryMap[0].endAddress != 0) {
        __NOP();
    }
    __NOP();
}

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
 * @param itemHandler
 * @param dataBuffer
 * @param bufferSize
 * @param startBlock
 * @param numOfBlocks
 * @return
 */
etl::expected<void, Error> eMMC::getItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize, uint32_t startBlock, uint32_t numOfBlocks) {

    if (bufferSize < itemHandler.size - (startBlock * memoryPageSize)) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    if ((startBlock * memoryPageSize) + (numOfBlocks * memoryPageSize) > itemHandler.size) {
        if (numOfBlocks > 1) {
            for (int i = 0; i < numOfBlocks - 1; i++) {
                auto status = eMMC::readBlockEMMC(dataBuffer + (i * memoryPageSize), itemHandler.startAddress + ((i + startBlock) * memoryPageSize), 1);
                // handle errors
                if (!status.has_value()) {
                    return status;
                }
            }
        }
        //compensate for item sizes not multiple of page size
        uint8_t numberOfLastBytes = itemHandler.size % memoryPageSize;
        if (numberOfLastBytes > 0) {
            uint8_t lastPage[memoryPageSize];
            uint32_t lastPageAdress = itemHandler.startAddress + ((itemHandler.size / memoryPageSize) * memoryPageSize);
            auto status = eMMC::readBlockEMMC(lastPage, lastPageAdress, 1);
            // handle errors
            if (!status.has_value()) {
                return status;
            }
            for (int i = 0; i < numberOfLastBytes; i++) {
                dataBuffer[((numOfBlocks - 1) * memoryPageSize) + i] = lastPage[i];
            }
        }
    } else {
        if (numOfBlocks > 0) {
            for (int i = 0; i < numOfBlocks; i++) {
                auto status = eMMC::readBlockEMMC(dataBuffer + (i * memoryPageSize), itemHandler.startAddress + ((i + startBlock) * memoryPageSize), 1);
                // handle errors
                if (!status.has_value()) {
                    return status;
                }
            }
        }
    }

    return {}; // success
}

/**
 *
 * @param itemHandler
 * @param dataBuffer
 * @param bufferSize
 * @return
 */
etl::expected<void, Error> eMMC::getItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize) {

    if (bufferSize < itemHandler.size) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    uint32_t continuousPages = itemHandler.size / memoryPageSize;
    if (continuousPages > 0) {
        for (int i = 0; i < continuousPages; i++) {
            auto status = eMMC::readBlockEMMC(dataBuffer + (i * memoryPageSize), itemHandler.startAddress + (i * memoryPageSize), 1);
            // handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    }

    //compensate for item sizes not multiple of page size
    uint8_t numberOfLastBytes = itemHandler.size % memoryPageSize;
    if (numberOfLastBytes > 0) {
        uint8_t lastPage[memoryPageSize];
        auto status = eMMC::readBlockEMMC(lastPage, itemHandler.startAddress + continuousPages, 1);
        // handle errors
        if (!status.has_value()) {
            return status;
        }
        for (int i = 0; i < numberOfLastBytes; i++) {
            dataBuffer[(continuousPages * memoryPageSize) + i] = lastPage[i];
        }
    }
    return {}; // success
}

etl::expected<void, Error> eMMC::storeItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize, uint32_t startBlock, uint32_t numOfBlocks) {

    if (bufferSize < itemHandler.size - (startBlock * memoryPageSize)) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    if ((startBlock * memoryPageSize) + (numOfBlocks * memoryPageSize) > itemHandler.size) {
        if (numOfBlocks > 1) {
            for (int i = 0; i < numOfBlocks - 1; i++) {
                auto status = eMMC::writeBlockEMMC(dataBuffer + (i * memoryPageSize), itemHandler.startAddress + ((i + startBlock) * memoryPageSize), 1);
                // handle errors
                if (!status.has_value()) {
                    return status;
                }
            }
        }
        //compensate for item sizes not multiple of page size
        uint8_t numberOfLastBytes = itemHandler.size % memoryPageSize;
        if (numberOfLastBytes > 0) {
            uint8_t lastPage[memoryPageSize];
            for (int i = 0; i < numberOfLastBytes; i++) {
                lastPage[i] = dataBuffer[((numOfBlocks - 1) * memoryPageSize) + i];
            }
            uint32_t lastPageAdress = itemHandler.startAddress + ((itemHandler.size / memoryPageSize) * memoryPageSize);
            auto status = eMMC::writeBlockEMMC(lastPage, lastPageAdress, 1);
            // handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    } else {
        if (numOfBlocks > 0) {
            auto status = eMMC::writeBlockEMMC(dataBuffer, itemHandler.startAddress + (startBlock * memoryPageSize), numOfBlocks);
            // handle errors
            if (!status.has_value()) {
                return status;
            }
            // for(int i=0; i<numOfBlocks; i++) {
            //     auto status = eMMC::writeBlockEMMC(dataBuffer+(i*memoryPageSize),itemHandler.startAddress+((i+startBlock)*memoryPageSize),1);
            //     // handle errors
            //     if(!status.has_value()) {
            //         return status;
            //     }
            // }
        }
    }

    return {}; // success
}

etl::expected<void, Error> eMMC::storeItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize) {

    if (bufferSize < itemHandler.size) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    uint32_t continuousPages = itemHandler.size / memoryPageSize;

    if (continuousPages > 0) {
        for (int i = 0; i < continuousPages; i++) {
            auto status = eMMC::writeBlockEMMC(dataBuffer + (i * memoryPageSize), itemHandler.startAddress + (i * memoryPageSize), 1);
            // handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    }

    //compensate for item sizes not multiple of page size
    uint8_t numberOfLastBytes = itemHandler.size % memoryPageSize;
    if (numberOfLastBytes > 0) {
        uint8_t lastPage[memoryPageSize];
        for (int i = 0; i < numberOfLastBytes; i++) {
            lastPage[i] = dataBuffer[(continuousPages * memoryPageSize) + i];
        }
        auto status = eMMC::writeBlockEMMC(lastPage, itemHandler.startAddress + continuousPages, 1);
        // handle errors
        if (!status.has_value()) {
            return status;
        }
    }
    return {}; // success
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
