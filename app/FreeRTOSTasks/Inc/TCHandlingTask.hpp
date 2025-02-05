#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include <queue.h>

class TCHandlingTask : public Task {
public:

    TCHandlingTask() : Task("TC Handling Task"){}
    [[noreturn]] void execute();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<TCHandlingTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 5000;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<TCHandlingTask> tcHandlingTask;