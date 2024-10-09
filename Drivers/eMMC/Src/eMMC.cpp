#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "eMMC.hpp"
#include "Logger.hpp"


using namespace eMMC;

extern MMC_HandleTypeDef hmmc1;

uint8_t eMMC::initializeEMMC(){
    HAL_GPIO_WritePin(MMC_EN_GPIO_Port, MMC_EN_Pin, static_cast<GPIO_PinState>(RESET));
    LOG_DEBUG << "MMC_EN_GPIO_Port ENABLED";
    vTaskDelay(1500);
    HAL_GPIO_WritePin(MEM_SEL_GPIO_Port, MEM_SEL_Pin, static_cast<GPIO_PinState>(SET));
    LOG_DEBUG << "MEM_SEL_GPIO_Port ENABLED";
    vTaskDelay(1500);
    HAL_GPIO_WritePin(MMC_RST_GPIO_Port, MMC_RST_Pin, static_cast<GPIO_PinState>(RESET));
    LOG_DEBUG << "MMC_RST_GPIO_Port ENABLED";
    vTaskDelay(1500);

    hmmc1.Instance = SDMMC1;
    hmmc1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hmmc1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_ENABLE;
    hmmc1.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hmmc1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
    hmmc1.Init.ClockDiv = 1;
    if (HAL_MMC_Init(&hmmc1) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

HAL_MMC_CardInfoTypeDef eMMC::checkEMMC(){
    HAL_MMC_CardInfoTypeDef mmc_info;
    HAL_MMC_GetCardInfo(&hmmc1, &mmc_info);
    return mmc_info;
}

HAL_StatusTypeDef eMMC::writeBlockEMMC(uint8_t* write_data, uint32_t block_address){
    return HAL_MMC_WriteBlocks(&hmmc1, write_data, block_address, 1, 1000);
}

HAL_StatusTypeDef eMMC::readBlockEMMC(uint8_t* read_data, uint32_t block_address){
    return HAL_MMC_ReadBlocks(&hmmc1, read_data, block_address, 1, 1000);
}

HAL_StatusTypeDef eMMC::eraseBlocksEMMC(uint32_t block_address_start, uint32_t block_address_end){
    return HAL_MMC_Erase(&hmmc1, block_address_start, block_address_end);
}
