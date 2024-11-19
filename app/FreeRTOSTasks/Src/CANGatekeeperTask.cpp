#pragma once
#include "CANDriver.hpp"
#include "CANGatekeeperTask.hpp"
#include "TPProtocol.hpp"
#include "Logger.hpp"
#include "queue.h"
#include "eMMC.hpp"
// #include "CANDriver.cpp"
struct incomingFIFO incomingFIFO;
const uint32_t numberOfIncomingItemsInBuffer = 16;
uint8_t incomingBuffer[64 * numberOfIncomingItemsInBuffer];

CANGatekeeperTask::CANGatekeeperTask() : Task("CANGatekeeperTask") {
    CAN::initialize(0);


    incomingFIFO.buffer = incomingBuffer;
    incomingFIFO.NOfItems = numberOfIncomingItemsInBuffer;


    outgoingQueue = xQueueCreateStatic(PacketQueueSize, sizeof(CAN::Packet), outgoingQueueStorageArea,
                                       &outgoingQueueBuffer);
    vQueueAddToRegistry(outgoingQueue, "CAN Outgoing");

    incomingSFQueue = xQueueCreateStatic(PacketQueueSize, sizeof(CAN::Packet), incomingSFQueueStorageArea,
                                         &incomingSFQueueBuffer);
    vQueueAddToRegistry(incomingSFQueue, "CAN Incoming SF");

    incomingMFQueue = xQueueCreateStatic(PacketQueueSize, sizeof(CAN::Packet), incomingMFQueueStorageArea,
                                         &incomingMFQueueBuffer);
    vQueueAddToRegistry(incomingSFQueue, "CAN Incoming MF");
}

static uint32_t thread_notification;

void CANGatekeeperTask::execute() {
    CAN::Packet out_message = {};
    CAN::Packet in_message = {};

    taskHandle = xTaskGetCurrentTaskHandle();

    uint32_t ulNotifiedValue;
    uint8_t localMessageBuffer[1024];
    uint32_t localMessageBufferTailPointer = 0;
    uint32_t currentMFMessageSize = 0;

    while (true) {
        // LOG_DEBUG << "{START OF" << this->TaskName << "}";
        xTaskNotifyWait(0, 0, &ulNotifiedValue, portMAX_DELAY);


        while (getIncomingSFMessagesCount()) {
            // Get the message pointer from the queue
            xQueueReceive(incomingSFQueue, &in_message, portMAX_DELAY);

            // Extract metadata
            uint8_t metadata = in_message.dataPointer[0];
            uint8_t frameType = metadata >> 6;
            uint8_t payloadLength = metadata & 0x3F;

            // Check where which bus the message came from
            if (in_message.bus->Instance == FDCAN1) {
                __NOP();
            } else if (in_message.bus->Instance == FDCAN2) {
                __NOP();
            }

            if (frameType == CAN::TPProtocol::Frame::Single) {
                // Add frame to application layer queue
                for (int i = 1; i < payloadLength; i++) {
                    localMessageBuffer[i] = in_message.dataPointer[i];
                }
                // Write message to eMMC
                // todo
                // Add message to queue
                // todo
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::Final) {
                uint32_t previousTailPointer = localMessageBufferTailPointer;

                for (; localMessageBufferTailPointer < currentMFMessageSize; localMessageBufferTailPointer++) {
                    __NOP();
                    localMessageBuffer[localMessageBufferTailPointer] = in_message.dataPointer[localMessageBufferTailPointer - previousTailPointer + 1];
                }
                localMessageBufferTailPointer = 0;
                __NOP();
                // Write message to eMMC
                auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::CANMessages], localMessageBuffer, eMMC::memoryMap[eMMC::CANMessages].size, 0, 2);
                // todo
                // Add message to queue
                // todo
                __NOP();

            } else if (frameType == CAN::TPProtocol::Frame::Consecutive) {

                // Add frame to local buffer
                __NOP();

                for (uint32_t previousTailPointer = localMessageBufferTailPointer; localMessageBufferTailPointer < 63 + previousTailPointer; localMessageBufferTailPointer++) {
                    __NOP();
                    localMessageBuffer[localMessageBufferTailPointer] = in_message.dataPointer[localMessageBufferTailPointer - previousTailPointer + 1];
                }
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::First) {
                currentMFMessageSize = (in_message.dataPointer[0] & 0x3F) << 8;
                currentMFMessageSize = currentMFMessageSize | in_message.dataPointer[1];
                __NOP();
            }
            // CAN::TPProtocol::processSingleFrame(in_message);
        }
        // CAN::TPProtocol::processMultipleFrames();

        if (uxQueueMessagesWaiting(outgoingQueue)) {
            xQueueReceive(outgoingQueue, &out_message, portMAX_DELAY);
            CAN::send(out_message, ActiveBus);
        }
        LOG_DEBUG << "{END OF" << this->TaskName << "}";
    }
}