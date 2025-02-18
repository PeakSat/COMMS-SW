#pragma once
#include "stm32h7xx_hal.h"
#include "etl/expected.h"
#include <array>
#include <cstdint>
#include "FreeRTOS.h"
#include <semphr.h>


namespace eMMC {

    struct eMMCTransactionHandler {
        SemaphoreHandle_t eMMC_semaphore;
        bool WriteComplete = false;
        bool ReadComplete = false;
        bool ErrorOccured = false;
        bool transactionAborted = false;
        MMC_HandleTypeDef hmmcSnapshot;
        uint32_t transactionTimeoutPerBlock = 100; // ms
        uint32_t getSemaphoreTimeout = 1000;       //ms
    };
    extern eMMCTransactionHandler eMMCTransactionHandler;

    // Declare constants
    extern uint64_t memoryCapacity;
    extern const uint32_t memoryPageSize; // bytes

    // Define the enum using the MEMORY_ITEM macro from the definition file
#define MEMORY_ITEM(name, size) name,
    enum memoryItem {
#include "MemoryItems.def"
        memoryItemCount // This is automatically added after all items
    };
#undef MEMORY_ITEM

    struct memoryItemHandler {
        // memoryItemData();
        // explicit memoryItemData(uint32_t newSize);
        uint64_t size;
        uint64_t startAddress;
        uint64_t endAddress;
        memoryItemHandler() : size(0), startAddress(0), endAddress(0) {}
        explicit memoryItemHandler(uint32_t newSize)
            : size(newSize), startAddress(0), endAddress(0) {}
    };

    // Error status
    enum class Error : uint8_t {
        NO_ERRORS = 0,
        EMMC_READ_FAILURE,
        EMMC_WRITE_FAILURE,
        EMMC_ERASE_BLOCK_FAILURE,
        EMMC_INVALID_NUM_OF_BLOCKS,
        EMMC_INVALID_START_ADDRESS_ON_ERASE,
        EMMC_TRANSACTION_TIMED_OUT,
        EMMC_BUFFER_TOO_SMALL
    };

    extern std::array<memoryItemHandler, memoryItemCount> memoryMap;

    void eMMCMemoryInit();

    /**
     * 
     * @param write_data 
     * @param block_address 
     * @param numberOfBlocks
     * @return 
     */
    etl::expected<void, Error> writeBlockEMMC(uint8_t* write_data, uint32_t block_address, uint32_t numberOfBlocks);

    /**
     * 
     * @param read_data 
     * @param block_address 
     * @param numberOfBlocks
     * @return 
     */
    etl::expected<void, Error> readBlockEMMC(uint8_t* read_data, uint32_t block_address, uint32_t numberOfBlocks);

    /**
  *
  * @param itemHandler
  * @param dataBuffer
  * @param bufferSize
  * @param startBlock
  * @param numOfBlocks
  * @return
  */
    etl::expected<void, Error> getItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize, uint32_t startBlock, uint32_t numOfBlocks);

    /**
 *
 * @param itemHandler
 * @param dataBuffer
 * @param bufferSize
 * @return
 */
    etl::expected<void, Error> getItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize);

    /**
     *
     * @param itemHandler
     * @param dataBuffer
     * @param bufferSize
     * @param startBlock
     * @param numOfBlocks
     * @return
     */
    etl::expected<void, Error> storeItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize, uint32_t startBlock, uint32_t numOfBlocks);

    /**
     *
     * @param itemHandler
     * @param dataBuffer
     * @param bufferSize
     * @return
     */
    etl::expected<void, Error> storeItem(memoryItemHandler itemHandler, uint8_t* dataBuffer, uint32_t bufferSize);

    /**
     * 
     * @param block_address_start 
     * @param block_address_end 
     * @return 
     */
    etl::expected<void, Error> eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end);

} // namespace eMMC