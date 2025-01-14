
#include "TPProtocol.hpp"
#include "CANGatekeeperTask.hpp"
#include "Peripheral_Definitions.hpp"
#include <ApplicationLayer.hpp>
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
using namespace CAN;

CANTransactionHandler CAN_TRANSMIT_Handler;

void TPProtocol::processSingleFrame(const CAN::Packet& message) {
    TPMessage tpMessage;
    tpMessage.decodeId(message.id);

    if (not(tpMessage.idInfo.isMulticast or tpMessage.idInfo.destinationAddress == NodeID)) {
        return;
    }

    size_t messageSize = message.data[0] & 0b111111;
    for (size_t idx = 1; idx <= messageSize; idx++) {
        tpMessage.appendUint8(message.data[idx]);
    }
    //    parseMessage(tpMessage);
}


void TPProtocol::parseMessage(TPMessage& message) {
    uint8_t messageType = static_cast<Application::MessageIDs>(message.data[0]);
    switch (messageType) {
        case CAN::Application::SendParameters:
            // CAN::Application::parseSendParametersMessage(message);
            break;
        case CAN::Application::RequestParameters:
            // CAN::Application::parseRequestParametersMessage(message);
            break;
        case CAN::Application::PerformFunction:
            break; //todo: use ST[08] to execute the perform function command
        case CAN::Application::EventReport:
            break; //todo: use the Event Report service
        case CAN::Application::TMPacket:
            CAN::Application::parseTMMessage(message);
            break;
        case CAN::Application::TCPacket:
            // CAN::Application::parseTCMessage(message);
            break;
        case CAN::Application::CCSDSPacket:
            break; //todo send this to comms? idk
        case CAN::Application::Ping: {
            auto senderID = static_cast<CAN::NodeIDs>(message.idInfo.sourceAddress);
            auto senderName = CAN::Application::nodeIdToString.at(senderID);
            LOG_DEBUG << "Received ping from " << senderName.c_str();
            CAN::Application::sendPongMessage();
        } break;
        case CAN::Application::Pong: {
            auto senderID = static_cast<CAN::NodeIDs>(message.idInfo.sourceAddress);
            auto senderName = CAN::Application::nodeIdToString.at(senderID);
            LOG_DEBUG << "Received pong from " << senderName.c_str();
        } break;
        case CAN::Application::LogMessage: {
            auto senderID = static_cast<CAN::NodeIDs>(message.idInfo.sourceAddress);
            auto senderName = CAN::Application::nodeIdToString.at(senderID);
            String<ECSSMaxMessageSize> logSource = "Incoming Log from ";
            logSource.append(senderName);
            logSource.append(": ");
            auto logData = String<ECSSMaxMessageSize>(message.data.data() + 1, message.dataSize - 1);
            LOG_DEBUG << logSource.c_str() << logData.c_str();
        } break;
        default:
            // ErrorHandler::reportInternalError(ErrorHandler::UnknownMessageType);
            break;
    }
}

bool TPProtocol::createCANTPMessage(const TPMessage& message, bool isISR) {
    if (!createCANTPMessageWithRetry(message, isISR, 2)) {
        return 0;
    } else {
        //Change CAN bus
        if (activeBus == CAN::ActiveBus::Redundant) {
            activeBus = CAN::ActiveBus::Main;
            canGatekeeperTask->switchActiveBus(Main);
        } else {
            activeBus = CAN::ActiveBus::Redundant;
            canGatekeeperTask->switchActiveBus(Redundant);
        }
        if (!createCANTPMessageWithRetry(message, isISR, 2)) {
            return 0;
        } else {
            //Packet transmit fialure
            uint32_t can1error = HAL_FDCAN_GetError(&hfdcan1);
            uint32_t can2error = HAL_FDCAN_GetError(&hfdcan2);

            FDCAN_ErrorCountersTypeDef CAN1errorCounter;
            HAL_FDCAN_GetErrorCounters(&hfdcan1, &CAN1errorCounter);
            FDCAN_ErrorCountersTypeDef CAN2errorCounter;
            HAL_FDCAN_GetErrorCounters(&hfdcan2, &CAN2errorCounter);

            FDCAN_ProtocolStatusTypeDef CAN1ProtocolStatus;
            HAL_FDCAN_GetProtocolStatus(&hfdcan1, &CAN1ProtocolStatus);
            FDCAN_ProtocolStatusTypeDef CAN2ProtocolStatus;
            HAL_FDCAN_GetProtocolStatus(&hfdcan2, &CAN2ProtocolStatus);

            LOG_ERROR << "Packet transmit failure. CAN1 error:" << can1error << " CAN2 error: " << can2error;
            LOG_ERROR << "CAN1 Error counter rx: " << CAN1errorCounter.RxErrorCnt << "Error counter tx: " << CAN1errorCounter.TxErrorCnt;
            LOG_ERROR << "CAN1 Last error: " << CAN1ProtocolStatus.LastErrorCode;

            LOG_ERROR << "CAN2 Error counter rx: " << CAN2errorCounter.RxErrorCnt << "Error counter tx: " << CAN2errorCounter.TxErrorCnt;
            LOG_ERROR << "CAN2 Last error: " << CAN2ProtocolStatus.LastErrorCode;
            return 1;
        }
    }
}

bool TPProtocol::createCANTPMessageWithRetry(const TPMessage& message, bool isISR, uint32_t NoOfRetries) {
    for (uint32_t i = 0; i < NoOfRetries; i++) {
        if (!createCANTPMessageNoRetransmit(message, isISR)) {
            return 0;
        }
        if (i > 0) {
            //number of retransmits ++
            LOG_ERROR << "Retransmitted CAN packet";
        }
    }
    return 1;
}

bool TPProtocol::createCANTPMessageNoRetransmit(const TPMessage& message, bool isISR) {

    size_t messageSize = message.dataSize;
    uint32_t id = message.encodeId();

    if (message.data[0] == Application::ACK) {
        etl::array<uint8_t, CAN::MaxPayloadLength> data = {
            static_cast<uint8_t>(((Single << 6) & 0xFF) | (messageSize & 0b111111))};
        for (size_t idx = 0; idx < messageSize; idx++) {
            data.at(idx + 1) = message.data[idx];
        }
        canGatekeeperTask->send({id, data}, isISR);
        return false;
    }

    // First Frame
    xSemaphoreTake(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE, portMAX_DELAY);
    CAN_TRANSMIT_Handler.ACKReceived = false;
    {
        // 4 MSB bits is the Frame Type identifier and the 4 LSB are the leftmost 4 bits of the data length.
        uint8_t firstByte = (First << 6) | ((messageSize >> 8) & 0b111111);
        // Rest of the data length.
        uint8_t secondByte = messageSize & 0xFF;

        etl::array<uint8_t, CAN::MaxPayloadLength> firstFrame = {firstByte, secondByte};

        canGatekeeperTask->send({id, firstFrame}, isISR);
        xTaskNotifyGive(canGatekeeperTask->taskHandle);
    }

    // Consecutive Frames
    uint8_t totalConsecutiveFramesNeeded = ceil(static_cast<float>(messageSize) / UsableDataLength);
    for (uint8_t currentConsecutiveFrameCount = 1;
         currentConsecutiveFrameCount <= totalConsecutiveFramesNeeded; currentConsecutiveFrameCount++) {

        uint8_t firstByte = (Consecutive << 6);
        if (currentConsecutiveFrameCount == totalConsecutiveFramesNeeded) {
            firstByte = (Final << 6);
        }
        etl::array<uint8_t, CAN::MaxPayloadLength> consecutiveFrame = {firstByte};
        consecutiveFrame.at(1) = currentConsecutiveFrameCount;

        for (uint8_t idx = 0; idx < UsableDataLength; idx++) {
            consecutiveFrame.at(idx + 2) = message.data[idx + UsableDataLength * (currentConsecutiveFrameCount - 1)];
        }
	// Make sure the output buffers do not overflow // Added a small delay every 4 frames
        if (currentConsecutiveFrameCount % 4 == 3) { 
            vTaskDelay(1);
        }

        canGatekeeperTask->send({id, consecutiveFrame}, isISR);
        xTaskNotifyGive(canGatekeeperTask->taskHandle);
    }

    uint32_t startTime = xTaskGetTickCount();
    while (true) {
        vTaskDelay(1);
        // ACK received
        if (CAN_TRANSMIT_Handler.ACKReceived == true) {
            xSemaphoreGive(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE);
            LOG_DEBUG << "CAN ACK received";
            __NOP();
            return false;
        }

        // Transaction timed out
        if (xTaskGetTickCount() > (CAN_TRANSMIT_Handler.CAN_ACK_TIMEOUT + startTime)) {
            xSemaphoreGive(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE);
            LOG_DEBUG << "CAN ACK timeout";
            __NOP();
            return true;
        }
    }
}
