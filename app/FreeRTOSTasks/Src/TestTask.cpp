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
    vTaskDelay(5000);


    while (true) {

        GNSSDefinitions::StoredGNSSData data{};
        if (eMMCDataTailPointer > 0) {
            auto status = eMMC::getItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&data), 512, eMMCDataTailPointer - 1, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
