#pragma once

#include "CANDriver.hpp"
#include "Task.hpp"

#include <optional>

class CANTestTask : public Task {
private:
public:
    const static inline uint16_t TaskStackDepth = 7000;

    StackType_t taskStack[TaskStackDepth];

    void execute();

    CANTestTask() : Task("CAN Test") {
    }


    /**
     * Create freeRTOS Task
     */
    void createTask() {
        xTaskCreateStatic(vClassTask<CANTestTask>, this->TaskName, CANTestTask::TaskStackDepth, this,
                          tskIDLE_PRIORITY + 2, this->taskStack, &(this->taskBuffer));
    }
};

inline std::optional<CANTestTask> canTestTask;