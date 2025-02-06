#pragma once

#include "Task.hpp"
#include "etl/optional.h"

inline bool heartbeatReceived = true;

class HeartbeatTask : public Task {
public:
    void execute();

    HeartbeatTask() : Task("HEARTBEAT") {}

    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<HeartbeatTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    //todo: set a file for all task settings
    const static inline uint16_t DelayMs = 5000;
    const static inline uint16_t TaskStackDepth = 1000;
    const static inline uint8_t LoggerPrecision = 2;
    const static uint8_t MaxErrorStringSize = 25;
    const static uint8_t MaxSensorNameSize = 16;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<HeartbeatTask> heartbeatTask;
