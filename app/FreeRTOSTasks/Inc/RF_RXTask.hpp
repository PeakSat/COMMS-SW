#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "queue.h"
#include "etl/array.h"
#include "etl/optional.h"
#include <etl/expected.h>
#include "main.h"
#include <Frame.hpp>

#define MAGIC_NUMBER 4

inline QueueHandle_t incomingTCQueue;
inline StaticQueue_t incomingTCQueueBuffer;
constexpr uint8_t TCQueueSize = 50;
inline uint8_t incomingTCQueueStorageArea[TCQueueSize * sizeof(CAN::StoredPacket)] __attribute__((section(".dtcmram_incomingTC ")));
using namespace AT86RF215;


using namespace AT86RF215;
class RF_RXTask : public Task {
public:

    RF_RXTask() : Task("RF RX TASK"){}
    void ensureRxMode();
    [[noreturn]] void execute();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_RXTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 3,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 10000;
    /// Frequency in kHz
    constexpr static uint32_t FrequencyUHFRX = 401000;
    AT86RF215::Error error = NO_ERRORS;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<RF_RXTask> rf_rxtask;