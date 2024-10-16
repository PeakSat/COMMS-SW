#pragma once

#include "stm32h7xx_hal_mmc.h"
#include "etl/utility.h"
#include "etl/expected.h"
#include "etl/optional.h"
#include "etl/array.h"
#include <cstdint>
#include <type_traits>
#include <tuple>



namespace eMMC{
    // Error status
    enum class Error : uint8_t {
        NO_ERRORS = 0,
        EMMC_READ_FAILURE,
        EMMC_WRITE_FAILURE,
        EMMC_ERASE_BLOCK_FAILURE,
    };

    /**
     * 
     * @param write_data 
     * @param block_address 
     * @param NumberOfBlocks 
     * @return 
     */
    etl::expected<void, Error>  writeBlockEMMC(uint8_t* write_data, uint32_t block_address, uint32_t NumberOfBlocks);

    /**
     * 
     * @param read_data 
     * @param block_address 
     * @param NumberOfBlocks 
     * @return 
     */
    etl::expected<void, Error> readBlockEMMC(uint8_t* read_data, uint32_t block_address, uint32_t NumberOfBlocks);

    /**
     * 
     * @param block_address_start 
     * @param block_address_end 
     * @return 
     */
    etl::expected<void, Error>  eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end);
    
}