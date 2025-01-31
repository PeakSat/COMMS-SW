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
    // etl::array<uint16_t, CAN::TPMessageMaximumArguments> testPArameters = {OBDHParameters::debugCounterID, OBDHParameters::PCBTemperature1ID, OBDHParameters::PCBTemperature2ID,
    //                                                                        OBDHParameters::MCUTemperatureID, OBDHParameters::MCUInputVoltageID, OBDHParameters::MCUSystickID, OBDHParameters::MCUBootCounterID};
    // constexpr int start_id = 3000; // Starting ID
    // constexpr int end_id = 3017;   // Ending ID
    // constexpr int size = end_id - start_id;
    // etl::array<uint16_t, CAN::TPMessageMaximumArguments> EPSIDs{};
    //
    // for (uint16_t i = 0; i < size; ++i) {
    //     EPSIDs[i] = start_id + i;
    // }

    while (true) {

        CAN::Application::sendPingMessage(CAN::OBC, false);

        vTaskDelay(5000);

        CAN::Application::sendHeartbeatMessage();

        // //LOG_DEBUG << COMMSParameters::commsUHFBandPATemperature.getValue();
        // LOG_DEBUG << "REQUESTING TEMP PARAMETERS FROM OBC";
        // // CAN::Application::createRequestParametersMessage(CAN::OBC, false, testPArameters, false);
        // // // Message Generation
        // Message generateOneShotReport(HousekeepingService::ServiceType, HousekeepingService::MessageType::GenerateOneShotHousekeepingReport, Message::TC, 1);
        // uint32_t numbeOfStructs = 1;
        // ParameterReportStructureId structure_ids[numbeOfStructs] = {0}; // TODO: Add correct IDs @tsoupos
        //
        // generateOneShotReport.appendUint8(numbeOfStructs);
        // for (auto& id: structure_ids) {
        //     generateOneShotReport.append<ParameterReportStructureId>(id);
        // }
        //
        // // auto cobsEncoded = COBSencode<ECSSMaxMessageSize>(MessageParser::composeECSS(generateOneShotReport, size));
        // // LOG_DEBUG << "Generate COBS encoded: " << &cobsEncoded;
        // CAN::Application::createPacketMessage(CAN::OBC, false, MessageParser::composeECSS(generateOneShotReport, ECSSSecondaryTCHeaderSize + generateOneShotReport.dataSize), Message::PacketType::TC, false);

        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, testPArameters, false);
        // LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        // LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
