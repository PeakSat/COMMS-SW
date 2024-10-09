#pragma once

#include "Task.hpp"
#include "etl/array.h"
#include "etl/optional.h"


class eMMCTask : public Task {
private:
    const static inline uint16_t DelayMs = 60000;
    const static inline uint16_t TaskStackDepth = 2000;

    StackType_t taskStack[TaskStackDepth];
public:

    void execute();

    eMMCTask() : Task("eMMC Memory Functions") {}

    void createTask(){
        xTaskCreateStatic(vClassTask<eMMCTask>, this->TaskName,
                          eMMCTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                          this->taskStack, &(this->taskBuffer));
    }
};

inline etl::optional<eMMCTask> eMMCTask;
