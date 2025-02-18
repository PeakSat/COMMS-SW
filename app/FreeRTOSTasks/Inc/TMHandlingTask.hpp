#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include <queue.h>
#include "stm32h7xx_hal.h"
#include <Frame.hpp>
#include "Logger.hpp"

extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;

inline QueueHandle_t incomingTMQueue;
inline StaticQueue_t incomingTMQueueBuffer;
constexpr uint8_t incomingTMQueueSize = 50;
inline uint8_t incomingTMQueueStorageArea[incomingTMQueueSize * sizeof(CAN::StoredPacket)];

class TMHandling : public Task {
public:
   TMHandling() : Task("TM Parsing Task"){}
    [[noreturn]] void execute();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<TMHandling>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 5000;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<TMHandling> tmhandlingTask;