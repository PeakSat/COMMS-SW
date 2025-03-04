#include "Logger.hpp"
#include "TestTask.hpp"
#include "eMMC.hpp"
#include "PlatformParameters.hpp"
#include "ParameterService.hpp"
#include "ApplicationLayer.hpp"
#include "COBS.hpp"
#include "ServicePool.hpp"

#include <GNSS.hpp>
#include <GNSSDefinitions.hpp>
#include <HousekeepingService.hpp>
#include <RF_TXTask.hpp>
#include <ServicePool.hpp>

void TestTask::execute() {
    LOG_DEBUG << "TestTask::execute()";
    // ServicePool::functionManagement.include()
    // Services.functionManagement.include(String<ECSSFunctionNameLength>("foo").data(), &TestTask::myFunction);
    // functionManagement
    while (true) {
        myFunction();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void TestTask::myFunction() {
    LOG_DEBUG << "TestTask::myFunction()";
}