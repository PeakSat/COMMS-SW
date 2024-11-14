#pragma once
#include "CANDriver.hpp"
#include "CANGatekeeperTask.hpp"
#include "TPProtocol.hpp"
#include "Logger.hpp"
#include "queue.h"
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

    while (true) {
        // LOG_DEBUG << "{START OF" << this->TaskName << "}";
        xTaskNotifyWait(0, 0, &ulNotifiedValue, portMAX_DELAY);

        if (getIncomingSFMessagesCount()) {
            xQueueReceive(incomingSFQueue, &in_message, portMAX_DELAY);
            uint8_t buff[64];
            for (int i = 0; i < 64; i++) {
                buff[i] = in_message.dataPointer[i];
            }
            // if(in_message.bus->Instance==FDCAN1) {
            //     __NOP();
            // }else if(in_message.bus->Instance==FDCAN2) {
            //     __NOP();
            // }
            __NOP();
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