#pragma once
#include "CANDriver.hpp"
#include "CANGatekeeperTask.hpp"
#include "TPProtocol.hpp"
#include "Logger.hpp"
#include "queue.h"
// #include "CANDriver.cpp"


CANGatekeeperTask::CANGatekeeperTask() : Task("CANGatekeeperTask") {
    CAN::initialize(0);

    outgoingQueue = xQueueCreateStatic(FrameQueueSize, sizeof(CAN::Frame), outgoingQueueStorageArea,
                                       &outgoingQueueBuffer);
    vQueueAddToRegistry(outgoingQueue, "CAN Outgoing");

    incomingSFQueue = xQueueCreateStatic(FrameQueueSize, sizeof(CAN::Frame), incomingSFQueueStorageArea,
                                         &incomingSFQueueBuffer);
    vQueueAddToRegistry(incomingSFQueue, "CAN Incoming SF");

    incomingMFQueue = xQueueCreateStatic(FrameQueueSize, sizeof(CAN::Frame), incomingMFQueueStorageArea,
                                         &incomingMFQueueBuffer);
    vQueueAddToRegistry(incomingSFQueue, "CAN Incoming MF");
}

static uint32_t thread_notification;

void CANGatekeeperTask::execute() {
    CAN::Frame out_message = {};
    CAN::Frame in_message = {};

    taskHandle = xTaskGetCurrentTaskHandle();

    uint32_t ulNotifiedValue;

    while (true) {
        // LOG_DEBUG << "{START OF" << this->TaskName << "}";
        xTaskNotifyWait(0, 0, &ulNotifiedValue, portMAX_DELAY);

        if (getIncomingSFMessagesCount()) {
            xQueueReceive(incomingSFQueue, &in_message, portMAX_DELAY);
            CAN::TPProtocol::processSingleFrame(in_message);
        }
        CAN::TPProtocol::processMultipleFrames();

        if (uxQueueMessagesWaiting(outgoingQueue)) {
            xQueueReceive(outgoingQueue, &out_message, portMAX_DELAY);
            CAN::send(out_message, ActiveBus);
        }
        LOG_DEBUG << "{END OF" << this->TaskName << "}";
    }
}