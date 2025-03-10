#include "eMMC.hpp"

#include <Platform/Inc/PlatformParameters.hpp>

using namespace eMMC;
extern MMC_HandleTypeDef hmmc1;
extern CRC_HandleTypeDef hcrc;

namespace eMMC {
    // Define the global variables
    // const uint32_t memoryPageSize = 512;                 // bytes
    // uint64_t memoryCapacity = 0xE90E80 * 508; // bytes
    std::array<memoryQueueHandler, memoryQueueCount> memoryQueueMap;
    std::array<memoryItemHandler, memoryItemCount> memoryMap;
    struct eMMCTransactionHandler eMMCTransactionHandler;
} // namespace eMMC
/**
 *
 */
void eMMC::eMMCMemoryInit() {

    eMMCTransactionHandler.eMMC_semaphore = xSemaphoreCreateMutex();
    eMMCTransactionHandler.eMMC_writeCompleteSemaphore = xSemaphoreCreateBinary();
    eMMCTransactionHandler.eMMC_readCompleteSemaphore = xSemaphoreCreateBinary();

    // Initialize the memoryMap array using the sizes from MemoryItems.def
#define MEMORY_ITEM(name, size) memoryMap[name] = memoryItemHandler(size);
#define MEMORY_QUEUE(queue_name, sizeOfItem, numberOfItems, queueSize)
#include "MemoryItems.def"


#undef MEMORY_ITEM
#undef MEMORY_QUEUE

#define MEMORY_ITEM(name, size)
#define MEMORY_QUEUE(queue_name, sizeOfItem, numberOfItems, queueSize)                                                                             \
    queue_name##Queue = xQueueCreateStatic(numberOfItems, sizeof(memoryQueueItemHandler), queue_name##QueueStorageArea, &queue_name##QueueBuffer); \
    vQueueAddToRegistry(queue_name##Queue, " queue_name queue");                                                                                   \
    memoryQueueMap[queue_name] = memoryQueueHandler(sizeOfItem, numberOfItems, &queue_name##Queue);
#include "MemoryItems.def"


#undef MEMORY_ITEM
#undef MEMORY_QUEUE


    if (memoryMap[0].endAddress != 0) {
        __NOP();
    }
    uint64_t headPointer = 0;
    for (int i = 0; i < memoryItemCount; i++) {
        headPointer += memoryMap[i].size;
        if (headPointer > EMMC_SIZE_IN_BYTES) {
            // memory full
        } else {
            memoryMap[i].endAddress = headPointer;
            memoryMap[i].startAddress = headPointer - memoryMap[i].size;
        }

        // align for page size (512 bytes)
        headPointer += EMMC_PAGE_SIZE - (headPointer % EMMC_PAGE_SIZE);
    }

    for (int i = 0; i < memoryQueueCount; i++) {
        uint32_t pagesPerItem = memoryQueueMap[i].sizeOfItem / EMMC_PAGE_SIZE;
        if (pagesPerItem * EMMC_PAGE_SIZE < memoryQueueMap[i].sizeOfItem) { // if item size is not multiple of page size
            pagesPerItem++;
        }
        uint64_t wholeSizeInBytes = EMMC_PAGE_SIZE * pagesPerItem * memoryQueueMap[i].numberOfItems;
        headPointer += wholeSizeInBytes;
        if (headPointer > EMMC_SIZE_IN_BYTES) {
            // memory full
        } else {
            memoryQueueMap[i].startAddress = headPointer - wholeSizeInBytes;
            memoryQueueMap[i].firstPage = memoryQueueMap[i].startAddress / EMMC_PAGE_SIZE;
            memoryQueueMap[i].lastPage = memoryQueueMap[i].firstPage + (pagesPerItem * memoryQueueMap[i].numberOfItems);
        }
    }
    float eMMC_usage = (100 * static_cast<float>(headPointer)) / static_cast<float>(EMMC_SIZE_IN_BYTES);
    COMMSParameters::EMMC_USAGE.setValue(eMMC_usage);
    if (memoryMap[0].endAddress != 0) {
        __NOP();
    }
    __NOP();
}

/**
  *
  * @param queueHandler
  * @param itemHandler
  * @param buffer
  * @param bufferSize
  * @return
  */
etl::expected<void, Error> eMMC::getItemFromQueue(memoryQueueHandler queueHandler, memoryQueueItemHandler itemHandler, uint8_t* buffer, uint32_t bufferSize) {

    if (bufferSize < itemHandler.size) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    uint8_t localBuffer[EMMC_PAGE_SIZE];
    auto status = eMMC::readBlockEMMC(localBuffer, itemHandler.startPage * EMMC_PAGE_SIZE, 1);
    /// TODO: handle errors
    if (!status.has_value()) {
        return status;
    }

    uint32_t pageCounter = 0;
    for (int i = 0; i < itemHandler.size; i++) {
        buffer[i] = localBuffer[i - (pageCounter * EMMC_PAGE_SIZE)];
        if (i >= ((pageCounter + 1) * EMMC_PAGE_SIZE) - 1) {
            pageCounter++;
            status = eMMC::readBlockEMMC(localBuffer, EMMC_PAGE_SIZE * (pageCounter + itemHandler.startPage), 1);
            /// TODO: handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    }

    return {}; // success
}

/**
  *
  * @param queueHandler
  * @param itemHandler
  * @param buffer
  * @param bufferSize
  * @return
  */
etl::expected<void, Error> eMMC::storeItemInQueue(memoryQueueHandler queueHandler, memoryQueueItemHandler* itemHandler, uint8_t* buffer, uint32_t bufferSize) {
    if (bufferSize < itemHandler->size) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    itemHandler->startPage = queueHandler.tailPageOffset + queueHandler.firstPage;
    queueHandler.tailPageOffset++;
    if (queueHandler.tailPageOffset > queueHandler.lastPage - queueHandler.firstPage) {
        queueHandler.tailPageOffset = 0;
    }
    uint8_t localBuffer[EMMC_PAGE_SIZE];
    uint32_t pageCounter = 0;
    for (int i = 0; i < itemHandler->size; i++) {
        localBuffer[i - (pageCounter * EMMC_PAGE_SIZE)] = buffer[i];
        if (i >= (EMMC_PAGE_SIZE * (pageCounter + 1)) - 1 || i == (itemHandler->size - 1)) {
            auto status = eMMC::writeBlockEMMC(localBuffer, (itemHandler->startPage + pageCounter) * EMMC_PAGE_SIZE, 1);
            pageCounter++;
            /// TODO: handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    }
    return {}; // success
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
    for (int i = 0; i < EMMC_PAGE_SIZE; i++) {
        eMMC_WriteRead_Block_Buffer[i] = write_data[i];
    }
    // todo: set the CRC bytes
    uint32_t crc_value = HAL_CRC_Calculate(&hcrc, reinterpret_cast<uint32_t*>(eMMC_WriteRead_Block_Buffer), EMMC_PAGE_SIZE);
    for (int i = 0; i < 4; i++) {
        eMMC_WriteRead_Block_Buffer[EMMC_PAGE_SIZE + i] = static_cast<uint8_t>((crc_value >> (i * 8)) & 0xFF);
    }
    eMMCTransactionHandler.WriteComplete = false;
    eMMCTransactionHandler.ErrorOccured = false;
    eMMCTransactionHandler.transactionAborted = false;

    HAL_StatusTypeDef status = HAL_MMC_WriteBlocks_IT(&hmmc1, eMMC_WriteRead_Block_Buffer, block_address, numberOfBlocks);
    if (status != HAL_OK) {

        xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
        return etl::unexpected<Error>(Error::EMMC_WRITE_FAILURE);
    }

    if (xSemaphoreTake(eMMCTransactionHandler.eMMC_writeCompleteSemaphore, eMMCTransactionHandler.transactionTimeoutPerBlock) == pdFALSE) {
        xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
        LOG_ERROR << "eMMC transaction timed out";
        return etl::unexpected<Error>(Error::EMMC_TRANSACTION_TIMED_OUT);
    }
    xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
    if (eMMCTransactionHandler.WriteComplete == true) {
        return {};
    }
    // TODO: handle the error, check eMMCTransactionHandler.hmmcSnapshot for error messages.
    return etl::unexpected<Error>(Error::EMMC_WRITE_FAILURE);
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


    HAL_StatusTypeDef status = HAL_MMC_ReadBlocks_IT(&hmmc1, eMMC_WriteRead_Block_Buffer, block_address, numberOfBlocks);

    if (status != HAL_OK) {

        xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
        return etl::unexpected<Error>(Error::EMMC_READ_FAILURE);
    }

    if (xSemaphoreTake(eMMCTransactionHandler.eMMC_readCompleteSemaphore, eMMCTransactionHandler.transactionTimeoutPerBlock) == pdFALSE) {
        // todo: test the CRC bytes
        xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
        LOG_ERROR << "eMMC transaction timed out";
        return etl::unexpected<Error>(Error::EMMC_TRANSACTION_TIMED_OUT);
    }

    uint32_t crc_value = HAL_CRC_Calculate(&hcrc, reinterpret_cast<uint32_t*>(eMMC_WriteRead_Block_Buffer), EMMC_PAGE_SIZE);
    if (eMMCTransactionHandler.ReadComplete == true) {
        for (int i = 0; i < EMMC_PAGE_SIZE; i++) {
            read_data[i] = eMMC_WriteRead_Block_Buffer[i];
        }
        for (int i = 0; i < 4; i++) {
            if (eMMC_WriteRead_Block_Buffer[EMMC_PAGE_SIZE + i] != static_cast<uint8_t>((crc_value >> (i * 8)) & 0xFF)) {
                // error, wrong CRC
                xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
                return etl::unexpected<Error>(Error::EMMC_READ_CRC_ERROR);
            } else if (i == 3) {
                // correct CRC
                __NOP();
            }
        }
        xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
        return {};
    }
    xSemaphoreGive(eMMCTransactionHandler.eMMC_semaphore);
    // TODO: handle the error, check eMMCTransactionHandler.hmmcSnapshot for error messages.
    return etl::unexpected<Error>(Error::EMMC_READ_FAILURE);
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

    uint32_t storeDataSize = numOfBlocks * EMMC_PAGE_SIZE;
    if ((startBlock + numOfBlocks) * EMMC_PAGE_SIZE > itemHandler.size) {
        storeDataSize -= EMMC_PAGE_SIZE - (itemHandler.size % EMMC_PAGE_SIZE);
    }
    if (bufferSize < storeDataSize) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    if ((startBlock * EMMC_PAGE_SIZE) + (numOfBlocks * EMMC_PAGE_SIZE) > itemHandler.size) {
        if (numOfBlocks > 1) {
            for (uint32_t i = 0; i < numOfBlocks - 1; i++) {
                auto status = eMMC::readBlockEMMC(dataBuffer + (i * EMMC_PAGE_SIZE), itemHandler.startAddress + ((i + startBlock) * EMMC_PAGE_SIZE), 1);
                /// TODO: handle errors
                if (!status.has_value()) {
                    return status;
                }
            }
        }
        // compensate for item sizes not multiple of page size
        uint8_t numberOfLastBytes = itemHandler.size % EMMC_PAGE_SIZE;
        if (numberOfLastBytes > 0) {
            uint8_t lastPage[EMMC_PAGE_SIZE];
            uint32_t lastPageAdress = itemHandler.startAddress + ((itemHandler.size / EMMC_PAGE_SIZE) * EMMC_PAGE_SIZE);
            auto status = eMMC::readBlockEMMC(lastPage, lastPageAdress, 1);
            /// TODO: handle errors
            if (!status.has_value()) {
                return status;
            }
            for (int i = 0; i < numberOfLastBytes; i++) {
                dataBuffer[((numOfBlocks - 1) * EMMC_PAGE_SIZE) + i] = lastPage[i];
            }
        }
    } else {
        if (numOfBlocks > 0) {
            for (uint32_t i = 0; i < numOfBlocks; i++) {
                auto status = eMMC::readBlockEMMC(dataBuffer + (i * EMMC_PAGE_SIZE), itemHandler.startAddress + ((i + startBlock) * EMMC_PAGE_SIZE), 1);
                /// TODO: handle errors
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
    uint32_t continuousPages = itemHandler.size / EMMC_PAGE_SIZE;
    if (continuousPages > 0) {
        for (uint32_t i = 0; i < continuousPages; i++) {
            auto status = eMMC::readBlockEMMC(dataBuffer + (i * EMMC_PAGE_SIZE), itemHandler.startAddress + (i * EMMC_PAGE_SIZE), 1);
            /// TODO: handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    }

    // compensate for item sizes not multiple of page size
    uint8_t numberOfLastBytes = itemHandler.size % EMMC_PAGE_SIZE;
    if (numberOfLastBytes > 0) {
        uint8_t lastPage[EMMC_PAGE_SIZE];
        auto status = eMMC::readBlockEMMC(lastPage, itemHandler.startAddress + continuousPages, 1);
        /// TODO: handle errors
        if (!status.has_value()) {
            return status;
        }
        for (int i = 0; i < numberOfLastBytes; i++) {
            dataBuffer[(continuousPages * EMMC_PAGE_SIZE) + i] = lastPage[i];
        }
    }
    return {}; // success
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
etl::expected<void, Error> eMMC::storeItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize, uint32_t startBlock, uint32_t numOfBlocks) {

    uint32_t storeDataSize = numOfBlocks * EMMC_PAGE_SIZE;
    if ((startBlock + numOfBlocks) * EMMC_PAGE_SIZE > itemHandler.size) {
        storeDataSize -= EMMC_PAGE_SIZE - (itemHandler.size % EMMC_PAGE_SIZE);
    }
    if (bufferSize < storeDataSize) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    if ((startBlock * EMMC_PAGE_SIZE) + (numOfBlocks * EMMC_PAGE_SIZE) > itemHandler.size) {
        if (numOfBlocks > 1) {
            for (uint32_t i = 0; i < numOfBlocks - 1; i++) {
                auto status = eMMC::writeBlockEMMC(dataBuffer + (i * EMMC_PAGE_SIZE), itemHandler.startAddress + ((i + startBlock) * EMMC_PAGE_SIZE), 1);
                // handle errors
                if (!status.has_value()) {
                    return status;
                }
            }
        }
        //compensate for item sizes not multiple of page size
        uint8_t numberOfLastBytes = itemHandler.size % EMMC_PAGE_SIZE;
        if (numberOfLastBytes > 0) {
            uint8_t lastPage[EMMC_PAGE_SIZE];
            for (int i = 0; i < numberOfLastBytes; i++) {
                lastPage[i] = dataBuffer[((numOfBlocks - 1) * EMMC_PAGE_SIZE) + i];
            }
            uint32_t lastPageAdress = itemHandler.startAddress + ((itemHandler.size / EMMC_PAGE_SIZE) * EMMC_PAGE_SIZE);
            auto status = eMMC::writeBlockEMMC(lastPage, lastPageAdress, 1);
            // handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    } else {
        if (numOfBlocks > 0) {
            // auto status = eMMC::writeBlockEMMC(dataBuffer, itemHandler.startAddress + (startBlock * EMMC_PAGE_SIZE), numOfBlocks);
            // // handle errors
            // if (!status.has_value()) {
            //     return status;
            // }
            for (uint32_t i = 0; i < numOfBlocks; i++) {
                auto status = eMMC::writeBlockEMMC(dataBuffer + (i * EMMC_PAGE_SIZE), itemHandler.startAddress + ((i + startBlock) * EMMC_PAGE_SIZE), 1);
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
etl::expected<void, Error> eMMC::storeItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize) {

    if (bufferSize < itemHandler.size) {
        return etl::unexpected<Error>(Error::EMMC_BUFFER_TOO_SMALL);
    }
    uint32_t continuousPages = itemHandler.size / EMMC_PAGE_SIZE;

    if (continuousPages > 0) {
        for (uint32_t i = 0; i < continuousPages; i++) {
            auto status = eMMC::writeBlockEMMC(dataBuffer + (i * EMMC_PAGE_SIZE), itemHandler.startAddress + (i * EMMC_PAGE_SIZE), 1);
            // handle errors
            if (!status.has_value()) {
                return status;
            }
        }
    }

    //compensate for item sizes not multiple of page size
    uint8_t numberOfLastBytes = itemHandler.size % EMMC_PAGE_SIZE;
    if (numberOfLastBytes > 0) {
        uint8_t lastPage[EMMC_PAGE_SIZE];
        for (int i = 0; i < numberOfLastBytes; i++) {
            lastPage[i] = dataBuffer[(continuousPages * EMMC_PAGE_SIZE) + i];
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
