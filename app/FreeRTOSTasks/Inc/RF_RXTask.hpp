#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "queue.h"
#include "etl/array.h"
#include "etl/optional.h"
#include <etl/expected.h>
#include "main.h"


using namespace AT86RF215;
class RF_RXTask : public Task {
public:
    RF_RXTask() : Task("RF RX TASK"){}
    void ensureRxMode();
    void execute();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_RXTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 3,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 5000;
    /// Frequency in kHz
    constexpr static uint32_t FrequencyUHFRX = 401000;
    Error error = NO_ERRORS;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<RF_RXTask> rf_rxtask;