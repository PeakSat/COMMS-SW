#pragma once
#include "stm32h7xx_hal.h"
#include "etl/expected.h"
#include <array>
#include <cstdint>
#include "FreeRTOS.h"
#include <semphr.h>


struct memoryQueueItemHandler {
    uint32_t size;
    uint32_t startPage;
};

#define MEMORY_ITEM(name, size)
#define MEMORY_QUEUE(queue_name, item_size, queue_size)                                                                                        \
    static inline uint8_t queue_name##QueueStorageArea[sizeof(memoryQueueItemHandler) * queue_size] __attribute__((section(".dtcmram_data"))); \
    inline QueueHandle_t queue_name##Queue;                                                                                                    \
    static inline StaticQueue_t queue_name##QueueBuffer;

#include "MemoryItems.def"
#undef MEMORY_ITEM
#undef MEMORY_QUEUE

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
#define MEMORY_QUEUE(name, item_size, queue_size)
    enum memoryItem {
#include "MemoryItems.def"
        memoryItemCount // This is automatically added after all items
    };
#undef MEMORY_ITEM
#undef MEMORY_QUEUE

#define MEMORY_ITEM(name, size)
#define MEMORY_QUEUE(queue_name, item_size, queue_size) queue_name,
    enum memoryQueue {
#include "MemoryItems.def"
        memoryQueueCount // This is automatically added after all items
    };
#undef MEMORY_ITEM
#undef MEMORY_QUEUE

    struct memoryQueueHandler {
        QueueHandle_t* queue;
        uint32_t sizeOfItem;
        uint32_t numberOfItems;
        uint32_t tailPageOffset;
        uint32_t firstPage;
        uint32_t lastPage;
        uint64_t startAddress;
        memoryQueueHandler() : sizeOfItem(0), queue(NULL), tailPageOffset(0), numberOfItems(0), startAddress(0), firstPage(0), lastPage(0) {}
        explicit memoryQueueHandler(uint32_t newSize, uint32_t newNumberOfItems, QueueHandle_t* newQueue)
            : sizeOfItem(newSize), numberOfItems(newNumberOfItems), queue(newQueue), tailPageOffset(0), startAddress(0), firstPage(0), lastPage(0) {}
    };
    struct memoryItemHandler {
        // memoryItemData();
        // explicit memoryItemData(uint32_t newSize);
        uint32_t size;
        uint32_t startAddress;
        uint32_t endAddress;
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
    extern std::array<memoryQueueHandler, memoryQueueCount> memoryQueueMap;

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
      * @param queueHandler
      * @param itemHandler
      * @param buffer
      * @param bufferSize
      * @return
      */
    etl::expected<void, Error> getItemFromQueue(memoryQueueHandler queueHandler, memoryQueueItemHandler itemHandler, uint8_t* buffer, uint32_t bufferSize);

    /**
      *
      * @param queueHandler
      * @param itemHandler
      * @param buffer
      * @param bufferSize
      * @return
      */
    etl::expected<void, Error> storeItemInQueue(memoryQueueHandler queueHandler, memoryQueueItemHandler* itemHandler, uint8_t* buffer, uint32_t bufferSize);

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