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
#include <TPProtocol.hpp>

void TestTask::execute() {
    LOG_DEBUG << "TestTask::execute()";
    vTaskDelay(1000);

    //get item ready to be stored to eMMC queue
    memoryQueueItemHandler enqueueHandler{};
    enqueueHandler.size = 520;
    uint8_t testData[enqueueHandler.size];
    for (int i = 0; i < enqueueHandler.size; i++) {
        testData[i] = i % 10;
    }

    while (true) {

        //send item to test queue in eMMC
        eMMC::storeItemInQueue(eMMC::memoryQueueMap[eMMC::testData], &enqueueHandler, testData, enqueueHandler.size);
        xQueueSendToBack(testDataQueue, &enqueueHandler, NULL);

        vTaskDelay(100);

        //get item from test queue in eMMC
        memoryQueueItemHandler dequeueHandler{};
        xQueueReceive(testDataQueue, &dequeueHandler, portMAX_DELAY);
        uint8_t receivedTestData[dequeueHandler.size];
        eMMC::getItemFromQueue(eMMC::memoryQueueMap[eMMC::testData], dequeueHandler, receivedTestData, dequeueHandler.size);

        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
