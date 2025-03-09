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
    const static inline uint16_t DelayMs = 5000;
    const static inline uint16_t TaskStackDepth = 5000;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<TestTask> testTask;
