#pragma once

#include "stm32h7xx_hal_mmc.h"

namespace eMMC{
    /*
     * Initialize eMMC instance
     * returns: uint8_t [0 on success, 1-3 on error]
     */
    uint8_t initializeEMMC();

    HAL_MMC_CardInfoTypeDef checkEMMC();

    HAL_StatusTypeDef writeBlockEMMC(uint8_t* write_data, uint32_t block_address);
    HAL_StatusTypeDef readBlockEMMC(uint8_t* read_data, uint32_t block_address);
    HAL_StatusTypeDef eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end);

}