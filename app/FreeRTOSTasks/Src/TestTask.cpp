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
    uint8_t eccsBuffer[1024];
    uint8_t ecssbuf[9] = {0x23, 0x20, 0x3, 0x1B, 0, 0x1, 0x1, 0, 0};
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
        // auto x = (MessageParser::composeECSS(generateOneShotReport, size));
        // LOG_DEBUG << "Generate COBS encoded: " << &cobsEncoded;
        // auto cobsDecodedMessage = COBSdecode<1024>(cobsEncoded);
        // uint8_t messageLength = cobsDecodedMessage.size();
        // uint8_t* ecssTCBytes = reinterpret_cast<uint8_t*>(cobsDecodedMessage.data());
        //
        // for (int i = 0; i < messageLength; i++) {
        //     eccsBuffer[i] = ecssTCBytes[i];
        // }
        __NOP();
        // CAN::Application::createPacketMessage(CAN::OBC, false, MessageParser::composeECSS(generateOneShotReport, ECSSSecondaryTCHeaderSize + generateOneShotReport.dataSize), Message::PacketType::TC, false);

        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, testPArameters, false);
        // LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        // LOG_DEBUG << "REQUESTING EPS PARAMETERS FROM OBC" ;
        // CAN::Application::createRequestParametersMessage(CAN::OBC, false, EPSIDs, false);
        // etl::string<9> string;
        // etl::format_spec format;
        // for (uint8_t i : ecssbuf)
        //     etl::to_string(i, string, format, false);
        // LOG_DEBUG << string;
        // CAN::Application::createPacketMessage(CAN::OBC, false, , Message::TC, false);
        // CAN::TPMessage message = {{CAN::NodeID, CAN::OBC, false}};
        //
        // // message.appendUint8(CAN::Application::MessageIDs::TCPacket);
        // for (int i = 0; i < 9; i++) {
        //     message.appendUint8(ecssbuf[i]);
        // }
        // CAN::TPProtocol::createCANTPMessage(message, false);
        // LOG_DEBUG << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        // vTaskDelay(pdMS_TO_TICKS(40));
        // LOG_DEBUG << "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
        // vTaskDelay(pdMS_TO_TICKS(40));
        // LOG_DEBUG << "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
        // vTaskDelay(pdMS_TO_TICKS(40));
        // LOG_DEBUG << "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";
        // vTaskDelay(pdMS_TO_TICKS(40));
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
