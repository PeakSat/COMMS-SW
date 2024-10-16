#include "Logger.hpp"
#include "eMMCTask.hpp"
#include "eMMC.hpp"


void eMMCTask::execute() {

    uint8_t data_buff[1024];
    for(uint32_t i = 0; i < 1024; i++)
        data_buff[i] = i % 250;

    uint32_t block_address_a = 5;
    uint8_t read_data_buff[1024];

    while(true){
        uint8_t error = 0;
        LOG_DEBUG << "Writing to block address: " << block_address_a;
        auto status = eMMC::writeBlockEMMC(data_buff, block_address_a,2);
        if(status.has_value()){
            // read was successful
        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
            LOG_ERROR << "Write HAL error, a";

        LOG_DEBUG << "Reading from block address: " << block_address_a;
        status = eMMC::readBlockEMMC(read_data_buff, block_address_a,2);
        if(status.has_value()){
            // read was successful
        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
            LOG_ERROR << "Read HAL error, a";

        for(uint32_t i = 0; i < 1024; i++){
            if(read_data_buff[i] != i % 250)
                error = 1;
        }
        if(!error)
            LOG_INFO << "eMMC write and read op completed with success";
        else
            LOG_ERROR << "eMMC read_data_buff contains wrong values";
        // erase the first block
        status = eMMC::eraseBlocksEMMC(block_address_a,block_address_a);
        // wait for the erase to finish
        vTaskDelay(10);
        if(status.has_value()){
            // erase was successful
        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
                LOG_ERROR << "Erase HAL error";
        // check the erase function with read
        status = eMMC::readBlockEMMC(read_data_buff, block_address_a,1);
        if(status.has_value()){
            // read was successful
        }else if(status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
                LOG_ERROR << "Read HAL error after erase";
        error = 0;
        for(uint32_t i = 0; i < 512; i++){
            if(read_data_buff[i] != 0)
                error = 1;
        }
        if(error)
            LOG_ERROR << "Error in erasing";
        else
            LOG_INFO << "Erase successful";
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}