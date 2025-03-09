#pragma once
#include "Task.hpp"
#include "etl/optional.h"

#include <Message.hpp>
#include <queue.h>
#include <stm32h7xx_hal.h>

#define OBC_APPLICATION_ID 1
#define TTC_APPLICATION_ID 2


extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;

inline uint8_t TC_UART_BUF[1024]{};

inline StaticQueue_t incomingTCUARTQueueBuffer{};
constexpr uint8_t TCUARTQueueSize = 10;
inline uint8_t ECSS_TC_BUF[1024]{};
inline constexpr uint8_t TCUARTItemSize = sizeof(uint8_t*);
inline uint8_t incomingTCUARTQueueStorageArea[TCUARTQueueSize * TCUARTItemSize]{};



class TCHandlingTask : public Task {
public:
    uint8_t* tc_buf_dma_pointer = nullptr;
    uint16_t size = 0;
    bool tc_uart_var = false;
    bool tc_rf_rx_var = false;
    QueueHandle_t TCUARTQueueHandle{};
    uint8_t* send_to_tc_queue{};
    static void startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size);
    void logParsedMessage(const Message& message);
    TCHandlingTask() : Task("TC Handling Task"){}
    [[noreturn]] void execute();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<TCHandlingTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 12000;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<TCHandlingTask> tcHandlingTask;