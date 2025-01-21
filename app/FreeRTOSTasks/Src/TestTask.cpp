#include "Logger.hpp"
#include "TestTask.hpp"
#include "eMMC.hpp"
#include "PlatformParameters.hpp"
#include "ParameterService.hpp"
#include "ApplicationLayer.hpp"
#include "COBS.hpp"
#include <HousekeepingService.hpp>
#include <RF_TXTask.hpp>
#include <ServicePool.hpp>

void TestTask::execute() {
    LOG_DEBUG << "TestTask::execute()";
    vTaskDelay(5000);
    etl::array<uint16_t, CAN::TPMessageMaximumArguments> OBCTempIDs = {5002};
    constexpr int start_id = 3000; // Starting ID
    constexpr int end_id = 3017;   // Ending ID
    constexpr int size = end_id - start_id;
    etl::array<uint16_t, CAN::TPMessageMaximumArguments> EPSIDs{};

    for (uint16_t i = 0; i < size; ++i) {
        EPSIDs[i] = start_id + i;
    }

    while (true) {
        LOG_DEBUG << COMMSParameters::commsUHFBandPATemperature.getValue();
        LOG_DEBUG << "REQUESTING TEMP PARAMETERS FROM OBC" ;
        CAN::Application::createRequestParametersMessage(CAN::OBC, false, OBCTempIDs, false);
        vTaskDelay(1000);
        LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
