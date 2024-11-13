#pragma once

#include "etl/vector.h"
#include "GNSSDefinitions.hpp"


class GNSSMessage {
private:
    typedef etl::vector<uint8_t, GNSSDefinitions::GNSSPayloadSize> Payload;
    etl::vector<uint8_t, 2> startOfSequence = {0xA0, 0xA1};
    etl::vector<uint8_t, 2> endOfSequence = {0x0D, 0x0A};

    static uint8_t calculateChecksum(const Payload& payload) {
        uint8_t checksum = 0;
        for (auto byte: payload) {
            checksum ^= byte;
        }
        return checksum;
    }

public:
    etl::vector<uint8_t, GNSSDefinitions::GNSSMessageSize> messageBody;
    uint8_t t = 0;
    GNSSMessage(uint16_t payloadLength, const GNSSDefinitions::Payload& payload) {
        messageBody.push_back(startOfSequence[0]);
        messageBody.push_back(startOfSequence[1]);
        messageBody.push_back(payloadLength >> 8);
        t = payloadLength;
        messageBody.push_back(t);
        for (uint8_t byte: payload) {
            messageBody.push_back(byte);
        }
        messageBody.push_back(calculateChecksum(payload));
        messageBody.push_back(endOfSequence[0]);
        messageBody.push_back(endOfSequence[1]);
    }

    GNSSMessage(uint16_t id, uint16_t payloadLength, const GNSSDefinitions::Payload& payload) {

        messageBody.push_back(startOfSequence[0]);
        messageBody.push_back(startOfSequence[1]);
        //        messageBody.push_back(((payloadLength + 2) >> 8) & 0xFF);
        //        messageBody.push_back((payloadLength + 2) & 0xFF);
        messageBody.push_back(payloadLength >> 8);
        t = payloadLength;
        messageBody.push_back(t);
        messageBody.push_back((id >> 8) & 0xFF);
        messageBody.push_back(id & 0xFF);
        for (uint8_t byte: payload) {
            messageBody.push_back(byte);
        }
        messageBody.push_back(0x63);
        messageBody.push_back(endOfSequence[0]);
        messageBody.push_back(endOfSequence[1]);
    }


    GNSSMessage() = default;
};