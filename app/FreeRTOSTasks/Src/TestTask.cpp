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
    vTaskDelay(12000);
    uint32_t eMMCPacketTailPointer = 0;
    uint8_t ecssbuf[512] = {0x23, 0x20, 0x3, 0x1B, 0, 0x1, 0x1, 0, 0};
    while (true) {
        // Write message to eMMC
        auto status = storeItem(eMMC::memoryMap[eMMC::COMMS_TC], ecssbuf, 512, eMMCPacketTailPointer, 1);
        // Add message to queue
        if (status.has_value()) {
            CAN::StoredPacket PacketToBeStored;
            PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
            eMMCPacketTailPointer += 2;
            PacketToBeStored.size = 9;
            LOG_DEBUG << "SEND TC FROM COMMS";
            xQueueSendToBack(outgoingTMQueue, &PacketToBeStored, 0);
            xTaskNotifyIndexed(rf_txtask->taskHandle, NOTIFY_INDEX_TRANSMIT, TC_COMMS, eSetBits);
        }
        else {
            LOG_ERROR << "Failed to send TC FROM COMMS";
        }
        // else {
        //     LOG_ERROR << status.error();
        // }
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}