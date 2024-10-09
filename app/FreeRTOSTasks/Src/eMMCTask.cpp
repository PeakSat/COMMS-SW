#include "Logger.hpp"
#include "main.h"
#include "eMMCTask.hpp"
#include "eMMC.hpp"

void eMMC_info_show(HAL_MMC_CardInfoTypeDef info){
    LOG_DEBUG << "*** EMMC INFO ***";
    LOG_DEBUG << "Card Type: " << info.CardType;
    LOG_DEBUG << "Card Class: " << info.Class;
    LOG_DEBUG << "Card Relative Address: " << info.RelCardAdd;
    LOG_DEBUG << "Capacity in Blocks: " << info.BlockNbr;
    LOG_DEBUG << "Block size: " << info.BlockSize;
}

void eMMCTask::execute() {

    while (eMMC::initializeEMMC() != 0) {
        vTaskDelay(10);
    };

    uint32_t block_address_a = 10;
    uint32_t block_address_b = 11;

    uint8_t data_buff[512];
    for(uint8_t i = 0; i < 50; i++){
        data_buff[i] = i;
    }
    HAL_StatusTypeDef  status;
    uint8_t read_data_buff[512];

    while(true){
        LOG_DEBUG <<"Reading from block address: " << block_address_a;
        eMMC::readBlockEMMC(read_data_buff, block_address_a);
        for(uint8_t i = 0; i < 50; i++){
            LOG_DEBUG << "I just read: " << read_data_buff[i];
        }
        LOG_DEBUG << "Reading from block address: " << block_address_b;
        eMMC::readBlockEMMC(read_data_buff, block_address_b);
        for(uint8_t i = 0; i < 50; i++){
            LOG_DEBUG << "I just read: " << read_data_buff[i];
        }
        LOG_DEBUG << "Writing to block address: " << block_address_a;
        status = eMMC::writeBlockEMMC(data_buff, block_address_a);
        LOG_DEBUG << "Status: " << status;
        LOG_DEBUG <<"Writing to block address: " << block_address_b;
        status = eMMC::writeBlockEMMC(data_buff, block_address_b);
        LOG_DEBUG << "Status: " << status;


        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}