#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "eMMC.hpp"
#include "Logger.hpp"
using namespace eMMC;

extern MMC_HandleTypeDef hmmc1;

/**
 * 
 * @param write_data 
 * @param block_address 
 * @param NumberOfBlocks 
 * @return 
 */
etl::expected<void, Error> eMMC::writeBlockEMMC(uint8_t* write_data, uint32_t block_address, uint32_t NumberOfBlocks) {
	if (HAL_MMC_WriteBlocks(&hmmc1, write_data, block_address, NumberOfBlocks, 1000) != HAL_OK) {
		return etl::unexpected<Error>(Error::EMMC_WRITE_FAILURE);
	}
	return {};
}

/**
 * 
 * @param read_data 
 * @param block_address 
 * @param NumberOfBlocks 
 * @return 
 */
etl::expected<void, Error> eMMC::readBlockEMMC(uint8_t* read_data, uint32_t block_address, uint32_t NumberOfBlocks) {
	if (HAL_MMC_ReadBlocks(&hmmc1, read_data, block_address, NumberOfBlocks, 1000) != HAL_OK) {
		return etl::unexpected<Error>(Error::EMMC_READ_FAILURE);
	}
	return {};
}

/**
 * 
 * @param block_address_start 
 * @param block_address_end 
 * @return 
 */
etl::expected<void, Error> eMMC::eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end) {
	if (HAL_MMC_Erase(&hmmc1, block_address_start, block_address_end) != HAL_OK) {
		return etl::unexpected<Error>(Error::EMMC_ERASE_BLOCK_FAILURE);
	}
	return {};
}
