#pragma once
#include "Task.hpp"
#include "etl/optional.h"
#include "TaskConfigs.hpp"


inline bool heartbeatReceived = true;

class HeartbeatTask : public Task {
public:
    [[noreturn]] void execute();
    HeartbeatTask() : Task("HEARTBEAT") {}
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<HeartbeatTask>, this->TaskName,HeartbeatTaskStack, this, HeartbeatTaskPriority, this->taskStack, &(this->taskBuffer));
    }
private:
    StackType_t taskStack[HeartbeatTaskStack]{};
};

inline etl::optional<HeartbeatTask> heartbeatTask;
