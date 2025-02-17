#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include <queue.h>
#include <stm32h7xx_hal.h>

extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;

inline StaticQueue_t incomingTCUARTQueueBuffer{};
constexpr uint8_t TCUARTQueueSize = 1;
inline uint8_t  TC_BUF[512]{};
inline uint8_t incomingTCUARTQueueStorageArea[TCUARTQueueSize * sizeof(uint8_t*)]{};

class TCHandlingTask : public Task {
public:
    uint8_t* tc_buf_dma_pointer = nullptr;
    uint16_t size = 0;
    QueueHandle_t TCUARTQueueHandle{};
    uint8_t* send_to_tc_queue{};
    static void startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size);
    TCHandlingTask() : Task("TC Handling Task"){}
    [[noreturn]] void execute();
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