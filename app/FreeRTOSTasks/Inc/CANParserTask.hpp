#pragma once

#include "CANDriver.hpp"
#include "Task.hpp"
#include <optional>
#include "RF_TXTask.hpp"

class CANParserTask : public Task {
public:
    const static inline uint16_t TaskStackDepth = 7000;

    StackType_t taskStack[TaskStackDepth];

    void execute();

    CANParserTask() : Task("CAN Parser") {
    }


    /**
     * Create freeRTOS Task
     */
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<CANParserTask>, this->TaskName, this->TaskStackDepth, this,
                                             tskIDLE_PRIORITY + 2, this->taskStack, &(this->taskBuffer));
    }
};

inline std::optional<CANParserTask> canParserTask;