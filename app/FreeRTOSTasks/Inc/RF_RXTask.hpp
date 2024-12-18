#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "queue.h"
#include "etl/array.h"
#include "etl/optional.h"
#include <etl/expected.h>
#include "main.h"
#define RECEIVE_FRAME_END (1 << 0)
#define AGC_HOLD (1 << 1)

using namespace AT86RF215;
class RF_RXTask : public Task {
public:
    RF_RXTask() : Task("RF RX TASK") {}
    void execute();
    etl::expected<void, Error> check_transceiver_connection();
    void createTask() {
        xTaskCreateStatic(vClassTask<RF_RXTask>, this->TaskName,
                          RF_RXTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                          this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 15000;
    /// frequency in kHz
    constexpr static uint32_t FrequencyUHFRX = 401500;
    AT86RF215::Error error;
    StackType_t taskStack[TaskStackDepth];
};

inline etl::optional<RF_RXTask> rf_rxtask;