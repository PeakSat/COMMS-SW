#include "Logger.hpp"
#include "TestTask.hpp"
#include "eMMC.hpp"
#include "PlatformParameters.hpp"
#include "ParameterService.hpp"
#include "ApplicationLayer.hpp"
#include "COBS.hpp"

#include <HousekeepingService.hpp>

void TestTask::execute() {
    vTaskDelay(1000);
    while (true) {
        LOG_DEBUG << "--------------Parameters Print----------------";
        LOG_DEBUG << "Parameter GNSS Temperature: " << COMMSParameters::commsGNSSTemperature.getValue();
        LOG_DEBUG << "Parameter UHF PA Temperature: " << COMMSParameters::commsUHFBandPATemperature.getValue();
        LOG_DEBUG << "Parameter PCB Temperature: " << COMMSParameters::commsPCBTemperature.getValue();

        // Message Generation
        Message generateOneShotReport(HousekeepingService::ServiceType, HousekeepingService::MessageType::GenerateOneShotHousekeepingReport, Message::TC, 1);
        uint32_t numbeOfStructs = 3;
        ParameterReportStructureId structure_ids[3] = {1, 2, 3}; // TODO: Add correct IDs @tsoupos
        generateOneShotReport.appendUint8(numbeOfStructs);
        for (auto& id: structure_ids) {
            generateOneShotReport.append<ParameterReportStructureId>(id);
        }

        auto cobsEncoded = COBSencode<ECSSMaxMessageSize>(generateOneShotReport.data.data(), generateOneShotReport.dataSize);

        LOG_DEBUG << "Generate COBS encoded: " << &cobsEncoded;
        CAN::Application::createPacketMessage(CAN::OBC, false, cobsEncoded, Message::PacketType::TC, false);

        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
