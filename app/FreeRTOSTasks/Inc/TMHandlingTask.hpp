#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include <queue.h>
#include "stm32h7xx_hal.h"
#include <Frame.hpp>
#include "Logger.hpp"
#include "TaskConfigs.hpp"

extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;

inline QueueHandle_t TMQueue;
inline StaticQueue_t TMQueueBuffer;
constexpr uint8_t TMQueueItemNum = 4;
constexpr size_t TMItemSize  = sizeof(TX_PACKET_HANDLER);
inline uint8_t TMQueueStorageArea[TMQueueItemNum * TMItemSize];
inline uint8_t TM_BUFF[1024];
class TMHandling : public Task {
public:
   TMHandling() : Task("TM Parsing Task"){}
    [[noreturn]] static void execute();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<TMHandling>, this->TaskName,
                                             TMHandlingTaskStack, this, TMHandlingTaskPriority,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    StackType_t taskStack[TMHandlingTaskStack]{};
};

inline etl::optional<TMHandling> tmhandlingTask;