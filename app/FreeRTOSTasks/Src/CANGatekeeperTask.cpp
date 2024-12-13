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
    CAN::Frame in_frame_handler = {};

    taskHandle = xTaskGetCurrentTaskHandle();

    uint32_t ulNotifiedValue;

    struct localPacketHandler {
        uint8_t Buffer[1024];
        uint32_t TailPointer = 0;
        uint32_t PacketSize = 0;
        uint8_t PacketID = 0;
    };
    struct localPacketHandler CAN1PacketHandler;
    struct localPacketHandler CAN2PacketHandler;

    // Variables for Rx message processing/storing
    uint8_t localPacketBuffer[1024];
    uint32_t localPacketBufferTailPointer = 0;
    uint32_t currentMFPacketSize = 0;
    uint8_t currenConsecutiveFrameCounter = 0;
    uint8_t currentMFPacketID = 0;
    uint32_t eMMCPacketTailPointer = 0;
    int j = 0;
    while (true) {
        // LOG_DEBUG << "{START OF" << this->TaskName << "}";
        xTaskNotifyWait(0, 0, &ulNotifiedValue, pdMS_TO_TICKS(1000));

        while (uxQueueMessagesWaiting(incomingFrameQueue)) {
            if (eMMCPacketTailPointer + 2 > eMMC::memoryMap[eMMC::CANMessages].size / 512) {
                eMMCPacketTailPointer = 0;
            }
            // Get the message pointer from the queue
            xQueueReceive(incomingFrameQueue, &in_frame_handler, portMAX_DELAY);

            struct localPacketHandler* CANPacketHandler;
            if (in_frame_handler.bus->Instance == FDCAN1) {
                CANPacketHandler = &CAN1PacketHandler;
            } else if (in_frame_handler.bus->Instance == FDCAN2) {
                CANPacketHandler = &CAN2PacketHandler;
            }

            // Extract metadata
            uint8_t metadata = in_frame_handler.pointerToData[0];
            uint8_t frameType = metadata >> 6;
            uint8_t payloadLength = metadata & 0x3F;
            if (frameType == CAN::TPProtocol::Frame::Single) {
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::First) {
                // debugCounter=0;
                CANPacketHandler->PacketSize = payloadLength << 8;
                CANPacketHandler->PacketSize = CANPacketHandler->PacketSize | in_frame_handler.pointerToData[1];
                CANPacketHandler->PacketSize -= 1; //compensate for ID byte
                // currenConsecutiveFrameCounter=0;
                CANPacketHandler->TailPointer = 0;
                __NOP();
            } else if (frameType == CAN::TPProtocol::Frame::Consecutive) {

                uint8_t FrameNumber = in_frame_handler.pointerToData[1] - 1;
                // Add frame to local buffer
                __NOP();

                for (uint32_t i = 0; i < (CAN::MaxPayloadLength - 2); i++) {
                    __NOP();
                    if (i + FrameNumber == 0) {
                        CANPacketHandler->PacketID = in_frame_handler.pointerToData[2];
                    } else {
                        CANPacketHandler->Buffer[(FrameNumber * (CAN::MaxPayloadLength - 2)) + i - 1] = in_frame_handler.pointerToData[i + 2];
                        CANPacketHandler->TailPointer = CANPacketHandler->TailPointer + 1;
                    }
                }
                __NOP();

            } else if (frameType == CAN::TPProtocol::Frame::Final) {
                if ((CANPacketHandler->PacketSize - CANPacketHandler->TailPointer) < (CAN::MaxPayloadLength - 2) && CANPacketHandler->PacketSize != 0) {
                    for (uint32_t i = 0; (CANPacketHandler->PacketSize > CANPacketHandler->TailPointer); i++) {

                        CANPacketHandler->Buffer[CANPacketHandler->TailPointer] = in_frame_handler.pointerToData[i + 2];
                        CANPacketHandler->TailPointer = CANPacketHandler->TailPointer + 1;
                    }
                    CAN::TPMessage message;
                    message.appendUint8(CANPacketHandler->PacketID);
                    for (int i = 0; i < CANPacketHandler->PacketSize; i++) {
                        message.appendUint8(CANPacketHandler->Buffer[i]);
                    }
                    CAN::TPProtocol::parseMessage(message);
                    // Write message to eMMC
                    auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::CANMessages], &CANPacketHandler->Buffer[0], 1024, eMMCPacketTailPointer, 2);
                    // Add message to queue
                    CAN::StoredPacket PacketToBeStored;
                    PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
                    eMMCPacketTailPointer += 2;
                    PacketToBeStored.Identifier = CANPacketHandler->PacketID;
                    PacketToBeStored.size = CANPacketHandler->PacketSize;
                    if (in_frame_handler.bus->Instance == FDCAN1) {
                        PacketToBeStored.CANInstance = CAN::CAN1;
                        __NOP();
                    } else if (in_frame_handler.bus->Instance == FDCAN2) {
                        PacketToBeStored.CANInstance = CAN::CAN2;
                        __NOP();
                    }

                    xQueueSendToBack(storedPacketQueue, &PacketToBeStored, NULL);
                } else {
                    // Message not received correctly
                    LOG_DEBUG << "DROPPED CAN MESSAGE";
                }
                __NOP();
                CANPacketHandler->TailPointer = 0;
            }
        }

        while (uxQueueMessagesWaiting(outgoingQueue)) {
            xQueueReceive(outgoingQueue, &out_message, portMAX_DELAY);
            CAN::send(out_message, ActiveBus);
        }
        LOG_DEBUG << "{END OF" << this->TaskName << "}";
    }
}