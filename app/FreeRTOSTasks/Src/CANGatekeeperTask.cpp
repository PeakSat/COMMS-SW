#pragma once
#include "CANDriver.hpp"
#include "CANGatekeeperTask.hpp"
#include "TPProtocol.hpp"
#include "Logger.hpp"
#include "queue.h"
#include "eMMC.hpp"
// #include "CANDriver.cpp"
struct incomingFIFO incomingFIFO;
uint8_t incomingBuffer[CANMessageSize * sizeOfIncommingFrameBuffer];

CANGatekeeperTask::CANGatekeeperTask() : Task("CANGatekeeperTask") {
    CAN::initialize(0);


    incomingFIFO.buffer = incomingBuffer;
    incomingFIFO.NOfItems = sizeOfIncommingFrameBuffer;


    outgoingQueue = xQueueCreateStatic(PacketQueueSize, sizeof(CAN::Packet), outgoingQueueStorageArea,
                                       &outgoingQueueBuffer);
    vQueueAddToRegistry(outgoingQueue, "CAN Outgoing");

    incomingSFQueue = xQueueCreateStatic(PacketQueueSize, sizeof(CAN::Packet), incomingSFQueueStorageArea,
                                         &incomingSFQueueBuffer);
    vQueueAddToRegistry(incomingSFQueue, "CAN Incoming SF");

    incomingMFQueue = xQueueCreateStatic(PacketQueueSize, sizeof(CAN::Packet), incomingMFQueueStorageArea,
                                         &incomingMFQueueBuffer);
    vQueueAddToRegistry(incomingSFQueue, "CAN Incoming MF");
    incomingFrameQueue = xQueueCreateStatic(sizeOfIncommingFrameBuffer, sizeof(CAN::Frame), incomingFrameQueueStorageArea,
                                            &incomingFrameQueueBuffer);
    vQueueAddToRegistry(incomingFrameQueue, "CAN Incoming Frame");
}

static uint32_t thread_notification;

void CANGatekeeperTask::execute() {
    CAN::Packet out_message = {};
    // CAN::Packet in_message = {};
    CAN::Frame in_frame = {};

    taskHandle = xTaskGetCurrentTaskHandle();

    uint32_t ulNotifiedValue;

    // Variables for Rx message processing/storing
    uint8_t localPacketBuffer[1024];
    uint32_t localPacketBufferTailPointer = 0;
    uint32_t currentMFPacketSize = 0;

    // Variables for eMMC storage handling
    uint32_t eMMCPacketTailPointer = 0;

    while (true) {
        // LOG_DEBUG << "{START OF" << this->TaskName << "}";
        xTaskNotifyWait(0, 0, &ulNotifiedValue, pdMS_TO_TICKS(1000));

        while (uxQueueMessagesWaiting(incomingFrameQueue)) {
            // Get the message pointer from the queue
            xQueueReceive(incomingFrameQueue, &in_frame, portMAX_DELAY);

            // Make sure the memory item size will not be exceeded
            if (eMMCPacketTailPointer + 2 > eMMC::memoryMap[eMMC::CANMessages].size / 512) {
                eMMCPacketTailPointer = 0;
            }

            // Extract metadata
            uint8_t metadata = in_frame.pointerToData[0];
            uint8_t frameType = metadata >> 6;
            uint8_t payloadLength = metadata & 0x3F;

            // Check which bus the message came from
            if (in_frame.bus->Instance == FDCAN1) {
                __NOP();
            } else if (in_frame.bus->Instance == FDCAN2) {
                __NOP();
            }

            if (frameType == CAN::TPProtocol::Frame::Single) {
                // Add frame to application layer queue
                for (int i = 1; i < payloadLength; i++) {
                    localPacketBuffer[i] = in_frame.pointerToData[i];
                }
                // Write message to eMMC
                // todo
                // Add message to queue
                // todo
                LOG_DEBUG << "CAN MF message received: " << payloadLength;
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::Final) {
                uint32_t previousTailPointer = localPacketBufferTailPointer;

                for (; localPacketBufferTailPointer < currentMFPacketSize; localPacketBufferTailPointer++) {
                    __NOP();
                    localPacketBuffer[localPacketBufferTailPointer] = in_frame.pointerToData[localPacketBufferTailPointer - previousTailPointer + 1];
                }
                localPacketBufferTailPointer = 0;
                __NOP();
                // Write message to eMMC
                auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::CANMessages], localPacketBuffer, 1024, eMMCPacketTailPointer += 2, 2);
                uint8_t testBuffer[1024];
                status = status = eMMC::getItem(eMMC::memoryMap[eMMC::CANMessages], testBuffer, 1024, eMMCPacketTailPointer - 2, 2);
                // todo
                // Add message to queue
                // todo
                __NOP();

            } else if (frameType == CAN::TPProtocol::Frame::Consecutive) {

                // Add frame to local buffer
                __NOP();

                for (uint32_t previousTailPointer = localPacketBufferTailPointer; localPacketBufferTailPointer < 63 + previousTailPointer; localPacketBufferTailPointer++) {
                    __NOP();
                    localPacketBuffer[localPacketBufferTailPointer] = in_frame.pointerToData[localPacketBufferTailPointer - previousTailPointer + 1];
                }
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::First) {
                currentMFPacketSize = payloadLength << 8;
                currentMFPacketSize = currentMFPacketSize | in_frame.pointerToData[1];
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