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
    vTaskDelay(5000);
    etl::array<uint16_t, CAN::TPMessageMaximumArguments> OBCTempIDs = {5002};
    // constexpr int start_id = 3000; // Starting ID
    // constexpr int end_id = 3614;   // Ending ID
    // constexpr int size = end_id - start_id;
    //
    // etl::array<uint16_t, CAN::TPMessageMaximumArguments> EPSIDs{};
    //
    // for (uint16_t i = 0; i < size; ++i) {
    //     EPSIDs[i] = start_id + i;
    // }
    LOG_INFO << "[TestTask] START ";
    while (true) {
        // LOG_DEBUG << "--------------Parameters Print----------------";
        // LOG_DEBUG << "Parameter GNSS Temperature: " << COMMSParameters::commsGNSSTemperature.getValue();
        // LOG_DEBUG << "Parameter UHF PA Temperature: " << COMMSParameters::commsUHFBandPATemperature.getValue();
        // LOG_DEBUG << "Parameter PCB Temperature: " << COMMSParameters::commsPCBTemperature.getValue();
        // Message Generation - parameter service
        /// auto &parameterManagement = Services.parameterManagement;
        // Message requestMessage;
        // Message Generation - housekeeping
        // Message generateOneShotReport(HousekeepingService::ServiceType, HousekeepingService::MessageType::GenerateOneShotHousekeepingReport, Message::TC, 1);
        // uint32_t numberOfStructs = 3;
        // ParameterReportStructureId structure_ids[3] = {1, 2, 3}; // TODO: Add correct IDs @tsoupos
        // generateOneShotReport.appendUint8(numberOfStructs);
        // for (auto& id: structure_ids) {
        // generateOneShotReport.append<ParameterReportStructureId>(id);
        // }
        /// housekeeping
        // auto cobsEncoded = COBSencode<ECSSMaxMessageSize>(generateOneShotReport.data.data(), generateOneShotReport.dataSize);
        /// parameter service
        CAN::Application::createRequestParametersMessage(CAN::OBC, false, OBCTempIDs, false);
        LOG_DEBUG << "REQUESTING TEMP PARAMETERS";
        LOG_DEBUG << COMMSParameters::commsUHFBandPATemperature.getValue();
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        // LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        // LOG_DEBUG << "Generate COBS encoded: " << &cobsEncoded;
        // CAN::Application::createPacketMessage(CAN::OBC, false, cobsEncoded, Message::PacketType::TC, false);

        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
