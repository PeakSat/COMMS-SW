#include "Logger.hpp"
#include "eMMCTask.hpp"
#include "eMMC.hpp"


void eMMCTask::execute() {

    uint8_t data_buff[eMMC::memoryMap[eMMC::test2].size];
    for (uint32_t i = 0; i < eMMC::memoryMap[eMMC::test2].size; i++)
        data_buff[i] = i % 250;

    uint32_t block_address_a = 5;
    uint8_t read_data_buff[eMMC::memoryMap[eMMC::test2].size];

    while (true) {


        LOG_DEBUG << "Writing to block address: " << block_address_a;
        auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::test2], data_buff, eMMC::memoryMap[eMMC::test2].size, 0, 3);

        if (status.has_value()) {
            // read was successful
        } else if (status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
            LOG_ERROR << "Write HAL error, a";

        LOG_DEBUG << "Reading from block address: " << block_address_a;
        status = eMMC::getItem(eMMC::memoryMap[eMMC::test2], read_data_buff, eMMC::memoryMap[eMMC::test2].size, 0, 3);
        if (status.has_value()) {
            // read was successful
        } else if (status.error() != eMMC::Error::NO_ERRORS)
            // handle the errors
            LOG_ERROR << "Read HAL error, a";

        uint8_t error = 0;
        for (uint32_t i = 0; i < eMMC::memoryMap[eMMC::test2].size; i++) {
            if (read_data_buff[i] != i % 250)
                error = 1;
        }
        if (!error)
            LOG_INFO << "eMMC write and read op completed with success";
        else
            LOG_ERROR << "eMMC read_data_buff contains wrong values";
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}