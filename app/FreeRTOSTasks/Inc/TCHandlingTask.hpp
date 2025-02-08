#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include <queue.h>
#include "stm32h7xx_hal.h"

#include <Frame.hpp>


extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;

inline StaticQueue_t incomingTCUARTQueueBuffer{};
constexpr uint8_t TCUARTQueueSize = 1;
inline uint8_t  TC_BUF[512]{};
inline uint8_t incomingTCUARTQueueStorageArea[TCUARTQueueSize * sizeof(uint8_t*)]{};

inline QueueHandle_t incomingTCQueue;
inline StaticQueue_t incomingTCQueueBuffer;
constexpr uint8_t incomingTCQueueSize = 50;
inline uint8_t incomingTCQueueStorageArea[incomingTCQueueSize * sizeof(CAN::StoredPacket)];

class TCHandlingTask : public Task {
public:
    uint8_t* tc_buf_dma_pointer = nullptr;
    uint16_t size = 0;
    QueueHandle_t TCUARTQueueHandle{};
    uint8_t* send_to_tc_queue{};
    TCHandlingTask() : Task("TC Handling Task"){}
    [[noreturn]] void execute();
    static void startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size);
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<TCHandlingTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 5000;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<TCHandlingTask> tcHandlingTask;