#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include "TaskConfigs.hpp"


class eMMCTask : public Task {
private:
    const static inline uint16_t DelayMs = 15000;

    StackType_t taskStack[eMMCTaskStack]{};

public:
    void execute();
    eMMCTask() : Task("eMMC Memory Functions") {}
    void createTask() {
        xTaskCreateStatic(vClassTask<eMMCTask>, this->TaskName,
                          eMMCTaskStack, this, eMMCTaskPriority, this->taskStack, &(this->taskBuffer));
    }
};

inline etl::optional<eMMCTask> eMMCTask;
