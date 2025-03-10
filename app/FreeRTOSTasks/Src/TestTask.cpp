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
#include <TCHandlingTask.hpp>

void TestTask::execute() {
    LOG_DEBUG << "TestTask::execute()";
    vTaskDelay(pdMS_TO_TICKS(5000));
    memoryQueueItemHandler rf_rx_tx_queue_handler{};
    uint8_t test_array[] = {24, 2, 1, 10, 0, 20, 47, 8, 1, 2, 5, 0, 2, 3, 0, 0, 0, 5, 0, 0, 0, 100, 0, 0, 0, 5};
    uint8_t RX_BUFF[1024]{};
    while (true) {
        LOG_DEBUG << "Sending TC to queue from test task";
        // tcHandlingTask->tc_rf_rx_var = true;
        // tcHandlingTask->send_to_tc_queue = test_array;
        // tcHandlingTask->size = sizeof(test_array)/sizeof(test_array[0]);
        // xQueueSendToBack(rf_rx_tcQueue, &tcHandlingTask->send_to_tc_queue, 0);
        // xTaskNotifyIndexed(tcHandlingTask->taskHandle, NOTIFY_INDEX_INCOMING_TC, (1 << 18), eSetBits);

        // for (int i = 0; i < sizeof(test_array) / sizeof(test_array[0]); i++) {
        //     RX_BUFF[i] = test_array[i];
        // }
        // rf_rx_tx_queue_handler.size = sizeof(test_array) / sizeof(test_array[0]);
        // auto status = eMMC::storeItemInQueue(eMMC::memoryQueueMap[eMMC::rf_rx_tc], &rf_rx_tx_queue_handler, RX_BUFF, rf_rx_tx_queue_handler.size);
        // if (status.has_value()) {
        //     if (rf_rx_tcQueue != nullptr) {
        //         xQueueSendToBack(rf_rx_tcQueue, &rf_rx_tx_queue_handler, 0);
        //         if (tcHandlingTask->taskHandle != nullptr) {
        //             tcHandlingTask->tc_rf_rx_var = true;
        //             xTaskNotifyIndexed(tcHandlingTask->taskHandle, NOTIFY_INDEX_INCOMING_TC, (1 << 19), eSetBits);
        //         } else {
        //             LOG_ERROR << "[RX] TC_HANDLING not started yet";
        //         }
        //     }
        // }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
