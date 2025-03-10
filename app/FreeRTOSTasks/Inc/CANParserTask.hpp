#pragma once

#include "CANDriver.hpp"
#include "Task.hpp"
#include <optional>
#include "RF_TXTask.hpp"

class CANParserTask : public Task {
public:

    StackType_t taskStack[CANParserTaskStack];

    void execute();

    CANParserTask() : Task("CAN Parser") {
    }


    /**
     * Create freeRTOS Task
     */
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<CANParserTask>, this->TaskName, CANParserTaskStack, this,
                                             CANParserTaskPriority, this->taskStack, &(this->taskBuffer));
    }
};

inline std::optional<CANParserTask> canParserTask;