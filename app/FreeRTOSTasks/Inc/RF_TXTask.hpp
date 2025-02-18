#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "etl/array.h"
#include "etl/optional.h"
#include <Frame.hpp>

#define MaxPacketLength 128

inline QueueHandle_t TXQueue;
inline StaticQueue_t outgoingTXQueueBuffer;
constexpr uint8_t outgoingTXQueueSize = 50;
inline uint8_t outgoingTXQueueStorageArea[outgoingTXQueueSize * sizeof(CAN::StoredPacket)] __attribute__((section(".dtcmram_outgoingTMQueueStorageArea")));
inline uint8_t TX_BUFF[1024] __attribute__((section(".dtcmram_tx_buff"), aligned(4)));
using namespace AT86RF215;



class RF_TXTask : public Task {
public:
    RF_TXTask() : Task("RF-TX Task") {}
    void print_state();
    [[noreturn]]void execute();
    void ensureTxMode();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_TXTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }
private:
    constexpr static uint16_t TaskStackDepth = 12000;
    /// Frequency in kHz
    constexpr static uint32_t FrequencyUHFTX = 401000;
    AT86RF215::Error error = NO_ERRORS;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<RF_TXTask> rf_txtask;