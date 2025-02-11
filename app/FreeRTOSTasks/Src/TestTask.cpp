#include "Logger.hpp"
#include "TestTask.hpp"
#include "eMMC.hpp"
#include "PlatformParameters.hpp"
#include "ParameterService.hpp"
#include "ApplicationLayer.hpp"
#include "COBS.hpp"

#include <GNSSDefinitions.hpp>
#include <HousekeepingService.hpp>
#include <RF_TXTask.hpp>
#include <ServicePool.hpp>

void TestTask::execute() {
    LOG_DEBUG << "TestTask::execute()";
    vTaskDelay(1000);
    // GNSSDefinitions::StoredGNSSData data{};
    // for (int i=0; i<100; i++) {
    //     auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&data), eMMC::memoryPageSize, i, 1);
    // }

    while (true) {


        // if (eMMCDataTailPointer > 0) {
        //     auto status = eMMC::getItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&data), 512, eMMCDataTailPointer - 1, 1);
        // }

        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
