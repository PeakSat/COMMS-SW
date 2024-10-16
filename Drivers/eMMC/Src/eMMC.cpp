
#include "eMMC.hpp"

using namespace eMMC;
extern MMC_HandleTypeDef hmmc1;

/**
 * 
 * @param write_data 
 * @param block_address 
 * @param numberOfBlocks
 * @return 
 */
etl::expected<void, Error> eMMC::writeBlockEMMC(uint8_t* write_data, uint32_t block_address, uint32_t numberOfBlocks){
    if(numberOfBlocks == 0)
        return etl::unexpected<Error>(Error::EMMC_INVALID_NUM_OF_BLOCKS);
    uint32_t timeout = 1000 * numberOfBlocks;
    if(HAL_MMC_WriteBlocks(&hmmc1, write_data, block_address, numberOfBlocks, timeout) != HAL_OK)
        return etl::unexpected<Error>(Error::EMMC_WRITE_FAILURE);
    return {}; // success
}

/**
 * 
 * @param read_data 
 * @param block_address 
 * @param numberOfBlocks
 * @return 
 */
etl::expected<void, Error> eMMC::readBlockEMMC(uint8_t* read_data, uint32_t block_address, uint32_t numberOfBlocks){
    if(numberOfBlocks == 0)
        return etl::unexpected<Error>(Error::EMMC_INVALID_NUM_OF_BLOCKS);
    uint32_t timeout = 1000 * numberOfBlocks;
    if(HAL_MMC_ReadBlocks(&hmmc1, read_data, block_address, numberOfBlocks, timeout) != HAL_OK)
        return etl::unexpected<Error>(Error::EMMC_READ_FAILURE);
    return {}; // success
}

/**
 * 
 * @param block_address_start 
 * @param block_address_end 
 * @return 
 */
etl::expected<void, Error> eMMC::eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end){
    if(block_address_start > block_address_end)
        return etl::unexpected<Error>(Error::EMMC_INVALID_START_ADDRESS_ON_ERASE);
    if(HAL_MMC_Erase(&hmmc1, block_address_start, block_address_end) != HAL_OK)
        return etl::unexpected<Error>(Error::EMMC_ERASE_BLOCK_FAILURE);
    return {}; // success
}
