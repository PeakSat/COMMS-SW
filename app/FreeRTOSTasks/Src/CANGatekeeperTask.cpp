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

    storedPacketQueue = xQueueCreateStatic(128, sizeof(CAN::StoredPacket), storedPacketQueueStorageArea,
                                           &storedPacketQueueBuffer);
    vQueueAddToRegistry(storedPacketQueue, "CAN stored packet");
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
    uint8_t currentMFPacketID = 0;

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

            if (frameType == CAN::TPProtocol::Frame::Single) {
                // Add frame to application layer queue
                uint8_t singleFrameBuffer[512];
                for (int i = 2; i < payloadLength + 1; i++) {
                    singleFrameBuffer[i - 2] = in_frame.pointerToData[i];
                }

                // Write message to eMMC
                auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::CANMessages], singleFrameBuffer, 512, eMMCPacketTailPointer, 1);

                // Add message to queue
                CAN::StoredPacket PacketToBeStored;
                PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
                eMMCPacketTailPointer += 2;
                PacketToBeStored.Identifier = in_frame.pointerToData[1];
                PacketToBeStored.size = static_cast<uint32_t>(payloadLength - 1);
                // Check which bus the message came from
                if (in_frame.bus->Instance == FDCAN1) {
                    PacketToBeStored.CANInstance = CAN::CAN1;
                    __NOP();
                } else if (in_frame.bus->Instance == FDCAN2) {
                    PacketToBeStored.CANInstance = CAN::CAN2;
                    __NOP();
                }
                xQueueSendToBack(storedPacketQueue, &PacketToBeStored, NULL);

                // Notify parser
                // todo
                // LOG_DEBUG << "CAN MF message received: " << payloadLength;
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::Final) {
                uint32_t previousTailPointer = localPacketBufferTailPointer;
                for (; localPacketBufferTailPointer < currentMFPacketSize + 1; localPacketBufferTailPointer++) {
                    __NOP();
                    localPacketBuffer[localPacketBufferTailPointer - 1] = in_frame.pointerToData[localPacketBufferTailPointer - previousTailPointer + 1];
                }
                localPacketBufferTailPointer = 0;
                __NOP();
                // Write message to eMMC
                auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::CANMessages], localPacketBuffer, 1024, eMMCPacketTailPointer, 2);
                // Add message to queue
                CAN::StoredPacket PacketToBeStored;
                PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
                eMMCPacketTailPointer += 2;
                PacketToBeStored.Identifier = currentMFPacketID;
                PacketToBeStored.size = currentMFPacketSize;
                // Check which bus the message came from
                if (in_frame.bus->Instance == FDCAN1) {
                    PacketToBeStored.CANInstance = CAN::CAN1;
                    __NOP();
                } else if (in_frame.bus->Instance == FDCAN2) {
                    PacketToBeStored.CANInstance = CAN::CAN2;
                    __NOP();
                }
                xQueueSendToBack(storedPacketQueue, &PacketToBeStored, NULL);

                // Notify parser
                // todo
                __NOP();

            } else if (frameType == CAN::TPProtocol::Frame::Consecutive) {

                // Add frame to local buffer
                __NOP();

                for (uint32_t previousTailPointer = localPacketBufferTailPointer; localPacketBufferTailPointer < 63 + previousTailPointer; localPacketBufferTailPointer++) {
                    __NOP();
                    if (localPacketBufferTailPointer == 0) {
                        currentMFPacketID = in_frame.pointerToData[localPacketBufferTailPointer - previousTailPointer + 1];
                    } else {
                        localPacketBuffer[localPacketBufferTailPointer - 1] = in_frame.pointerToData[localPacketBufferTailPointer - previousTailPointer + 1];
                    }
                }
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::First) {
                currentMFPacketSize = payloadLength << 8;
                currentMFPacketSize = currentMFPacketSize | in_frame.pointerToData[1];
                currentMFPacketSize -= 1; //compensate for ID byte
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