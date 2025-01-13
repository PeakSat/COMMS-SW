#include "Logger.hpp"
#include "TestTask.hpp"
#include "eMMC.hpp"
#include "PlatformParameters.hpp"
#include "ParameterService.hpp"

void TestTask::execute() {
    vTaskDelay(1000);
    while (true) {
        LOG_DEBUG << "--------------Parameters Print----------------";
        LOG_DEBUG << "Parameter GNSS Temperature: " << COMMSParameters::commsGNSSTemperature.getValue();
        LOG_DEBUG << "Parameter UHF PA Temperature: " << COMMSParameters::commsUHFBandPATemperature.getValue();
        LOG_DEBUG << "Parameter PCB Temperature: " << COMMSParameters::commsPCBTemperature.getValue();
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
