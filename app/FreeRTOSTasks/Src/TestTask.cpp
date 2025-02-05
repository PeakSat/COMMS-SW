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
    etl::array<uint16_t, CAN::TPMessageMaximumArguments> testPArameters = {OBDHParameters::debugCounterID, OBDHParameters::PCBTemperature1ID, OBDHParameters::PCBTemperature2ID,
                                                                           OBDHParameters::MCUTemperatureID, OBDHParameters::MCUInputVoltageID, OBDHParameters::MCUSystickID, OBDHParameters::MCUBootCounterID};
    constexpr int start_id = 3000; // Starting ID
    constexpr int end_id = 3017;   // Ending ID
    constexpr int size = end_id - start_id;
    // etl::array<uint16_t, CAN::TPMessageMaximumArguments> EPSIDs{};
    //
    // for (uint16_t i = 0; i < size; ++i) {
    //     EPSIDs[i] = start_id + i;
    // }
    uint8_t counter = 0;
    uint32_t eMMCPacketTailPointer = 0;
    uint8_t test_array[50];
    for (int i = 0; i < 50; i++) {
        test_array[i] = i;
    }

    while (true) {
        //LOG_DEBUG << COMMSParameters::commsUHFBandPATemperature.getValue();
        // LOG_DEBUG << "REQUESTING TEMP PARAMETERS FROM OBC";
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, testPArameters, false);
        // // Message Generation
        Message generateOneShotReport(HousekeepingService::ServiceType, HousekeepingService::MessageType::GenerateOneShotHousekeepingReport, Message::TC, 1);
        uint32_t numbeOfStructs = 1;
        ParameterReportStructureId structure_ids[numbeOfStructs] = {0}; // TODO: Add correct IDs @tsoupos

        generateOneShotReport.appendUint8(numbeOfStructs);
        for (auto& id: structure_ids) {
            generateOneShotReport.append<ParameterReportStructureId>(id);
        }

        auto cobsEncoded = COBSencode<ECSSMaxMessageSize>(MessageParser::composeECSS(generateOneShotReport, size));
        LOG_DEBUG << "Generate COBS encoded: " << &cobsEncoded;
        CAN::Application::createPacketMessage(CAN::OBC, false, MessageParser::composeECSS(generateOneShotReport, ECSSSecondaryTCHeaderSize + generateOneShotReport.dataSize), Message::PacketType::TC, false);

        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, testPArameters, false);
        // LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        // LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);

        // Write message to eMMC
        // PacketData packetTestData = rf_txtask->createRandomPacketData(MaxPacketLength);
        // counter++;
        // packetTestData.packet[0] = counter;
        // auto status = storeItem(eMMC::memoryMap[eMMC::COMMS_HOUSEKEEPING], test_array, 1024, eMMCPacketTailPointer, 2);
        // Add message to queue
        // CAN::StoredPacket PacketToBeStored;
        // PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
        // eMMCPacketTailPointer += 2;
        // PacketToBeStored.size = 50;
        // LOG_DEBUG << "SEND TM FROM COMMS";
        // xQueueSendToBack(outgoingTMQueue, &PacketToBeStored, 0);
        // xTaskNotifyIndexed(rf_txtask->taskHandle, NOTIFY_INDEX_TRANSMIT, TM_COMMS, eSetBits);
        // if (counter == 255)
           //  counter = 0;
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
