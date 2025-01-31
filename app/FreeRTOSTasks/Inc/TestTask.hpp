#pragma once

#include "Task.hpp"
#include "etl/optional.h"

class TestTask : public Task {
public:
    void execute();

    TestTask() : Task("ECSS TEST") {}

    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<TestTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    const static inline uint16_t DelayMs = 10000;
    const static inline uint16_t TaskStackDepth = 5000;
    const static inline uint8_t LoggerPrecision = 2;
    const static uint8_t MaxErrorStringSize = 25;
    const static uint8_t MaxSensorNameSize = 16;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<TestTask> testTask;
