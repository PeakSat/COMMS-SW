
#include "TPProtocol.hpp"
#include "CANGatekeeperTask.hpp"
#include "Peripheral_Definitions.hpp"

#include <ApplicationLayer.hpp>

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

void TPProtocol::processMultipleFrames() {
    TPMessage message;
    uint8_t incomingMessagesCount = canGatekeeperTask->getIncomingMFMessagesCount();
    uint16_t dataLength = 0;
    bool receivedFirstFrame = false;

    for (uint8_t messageCounter = 0; messageCounter < incomingMessagesCount; messageCounter++) {
        CAN::Packet frame = canGatekeeperTask->getFromMFQueue();
        auto frameType = static_cast<Frame>(frame.data[0] >> 6);

        if (not receivedFirstFrame) {
            if (frameType == First) {
                message.decodeId(frame.id);
                dataLength = ((frame.data[0] & 0b111111) << 8) | frame.data[1];
                receivedFirstFrame = true;
            }
        } else {
            uint8_t consecutiveFrameCount = frame.data[0] & 0b111111;
            //            if (not ErrorHandler::assertInternal(messageCounter == consecutiveFrameCount,
            //                                                 ErrorHandler::InternalErrorType::UnacceptablePacket)) { //TODO: Add a more appropriate enum value
            canGatekeeperTask->emptyIncomingMFQueue();
            return;
        }

        for (size_t idx = 1; idx < CAN::MaxPayloadLength; idx++) {
            message.appendUint8(frame.data[idx]);
            if (message.dataSize >= dataLength) {
                break;
            }
        }
    }

    //    if (not(message.idInfo.isMulticast or message.idInfo.destinationAddress == NodeID)) {
    //        return;
    //    }

    //    parseMessage(message);
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
            auto logData = String<ECSSMaxMessageSize>(message.data + 1, message.dataSize - 1);
            LOG_DEBUG << logSource.c_str() << logData.c_str();
        } break;
        default:
            // ErrorHandler::reportInternalError(ErrorHandler::UnknownMessageType);
            break;
    }
}


bool TPProtocol::createCANTPMessage(const TPMessage& message, bool isISR) {

    size_t messageSize = message.dataSize;
    uint32_t id = message.encodeId();
    // Data fits in a Single Frame
    if (messageSize <= UsableDataLength) {
        etl::array<uint8_t, CAN::MaxPayloadLength> data = {
            static_cast<uint8_t>(((Single << 6) & 0xFF) | (messageSize & 0b111111))};
        for (size_t idx = 0; idx < messageSize; idx++) {
            data.at(idx + 1) = message.data[idx];
        }
        if (message.data[0] == Application::ACK) {
            // Don't wait if it's an ack response
            canGatekeeperTask->send({id, data}, isISR);
        }else {
            xSemaphoreTake(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE, portMAX_DELAY);
            canGatekeeperTask->send({id, data}, isISR);
            xSemaphoreGive(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE);
        }

        return false;
    }

    // First Frame
    xSemaphoreTake(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE, portMAX_DELAY);
    CAN_TRANSMIT_Handler.ACKReceived=false;
    {
        // 4 MSB bits is the Frame Type identifier and the 4 LSB are the leftmost 4 bits of the data length.
        uint8_t firstByte = (First << 6) | ((messageSize >> 8) & 0b111111);
        // Rest of the data length.
        uint8_t secondByte = messageSize & 0xFF;

        etl::array<uint8_t, CAN::MaxPayloadLength> firstFrame = {firstByte, secondByte};

        canGatekeeperTask->send({id, firstFrame}, isISR);
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

        canGatekeeperTask->send({id, consecutiveFrame}, isISR);
    }

    uint32_t startTime = xTaskGetTickCount();
    while (true) {
        vTaskDelay(1);
        // ACK received
        if (CAN_TRANSMIT_Handler.ACKReceived == true) {
            xSemaphoreGive(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE);
            return false;
            LOG_DEBUG << "CAN ACK received";
            __NOP();
        }

        // Transaction timed out
        if (xTaskGetTickCount() > (CAN_TRANSMIT_Handler.CAN_ACK_TIMEOUT  + startTime)) {
            xSemaphoreGive(CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE);
            return true;
            LOG_DEBUG << "CAN ACK timeout";
            __NOP();
        }
    }
}
