#pragma once
#include "stm32h7xx_hal.h"
#include "etl/expected.h"


// #include <portmacro.h>

namespace eMMC{
    // Error status
    enum class Error : uint8_t {
        NO_ERRORS = 0,
        EMMC_READ_FAILURE,
        EMMC_WRITE_FAILURE,
        EMMC_ERASE_BLOCK_FAILURE,
        EMMC_INVALID_NUM_OF_BLOCKS,
        EMMC_INVALID_START_ADDRESS_ON_ERASE,
        EMMC_TRANSACTION_TIMED_OUT
    };

    /**
     * 
     * @param write_data 
     * @param block_address 
     * @param numberOfBlocks
     * @return 
     */
    etl::expected<void, Error>  writeBlockEMMC(uint8_t* write_data, uint32_t block_address, uint32_t numberOfBlocks);

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
     * @param block_address_start 
     * @param block_address_end 
     * @return 
     */
    etl::expected<void, Error>  eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end);
    
}