#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include <queue.h>
#include "stm32h7xx_hal.h"
#include <Frame.hpp>
#include "Logger.hpp"

extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;

inline QueueHandle_t TMQueue;
inline StaticQueue_t TMQueueBuffer;
constexpr uint8_t TMQueueItemNum = 50;
constexpr size_t TMItemSize  = sizeof(tm_handler);
inline uint8_t TMQueueStorageArea[TMQueueItemNum * TMItemSize];
inline uint8_t TM_BUFF[1024];
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
    constexpr static uint16_t TaskStackDepth = 10000;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<TMHandling> tmhandlingTask;