#pragma once
#include "Task.hpp"
#include "etl/optional.h"

inline bool heartbeatReceived = true;

class HeartbeatTask : public Task {
public:
    [[noreturn]] void execute();

    HeartbeatTask() : Task("HEARTBEAT") {}

    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<HeartbeatTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    const static inline uint16_t TaskStackDepth = 2500;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<HeartbeatTask> heartbeatTask;
