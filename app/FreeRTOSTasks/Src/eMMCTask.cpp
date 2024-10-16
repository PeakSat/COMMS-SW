#include "Logger.hpp"
#include "main.h"
#include "eMMCTask.hpp"
#include "eMMC.hpp"


void eMMCTask::execute() {

    uint32_t block_address_a = 5;

    uint8_t data_buff[1024];
    for(uint32_t i = 0; i < 1024; i++){
        data_buff[i] = i % 250;
    }

    uint8_t read_data_buff[1024];

    while(true){

        uint8_t error = 0;
        LOG_DEBUG << "Writing to block address: " << block_address_a;
        auto status = eMMC::writeBlockEMMC(data_buff, block_address_a,2);
        if(status.has_value()){
            // read was successful
        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
            LOG_DEBUG << "write hal error , a";

        LOG_DEBUG << "Reading from block address: " << block_address_a;
        status = eMMC::readBlockEMMC(read_data_buff, block_address_a,2);
        if(status.has_value()){
            // read was successful

        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
            LOG_DEBUG << "read hal error, a";

        for(uint32_t i = 0; i < 1024; i++){
            if(read_data_buff[i] != i % 250)
                error = 1;
        }
        if(!error)
            LOG_DEBUG << "eMMC good block_address_a";
        else
            LOG_DEBUG << "eMMC not good block_address_a";

        status = eMMC::eraseBlocksEMMC(block_address_a,block_address_a);
        vTaskDelay(10);
        if(status.has_value()){
            // erase was successful
        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
                LOG_DEBUG << "erase hal error";

        status = eMMC::readBlockEMMC(read_data_buff, block_address_a,1);
        if(status.has_value()){
            // read was successful
        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
                LOG_DEBUG << "read hall error after erase";
        error = 0;
        for(uint32_t i = 0; i < 512; i++){
            if(read_data_buff[i] != 0)
                error = 1;
        }
        if(error)
            LOG_DEBUG << "ERROR IN ERASING";
        else
            LOG_DEBUG << "ERASE SUCCESSFUL";
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}