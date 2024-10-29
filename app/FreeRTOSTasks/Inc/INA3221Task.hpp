#pragma once
// #include <portmacro.h>
#include <Task.hpp>
#include <etl/optional.h>


class INA3221Task : public Task {
private:
    const static inline uint16_t DelayMs = 6000;
    const static inline uint16_t TaskStackDepth = 4000;

    // extern I2C_HandleTypeDef INA3221I2CHandle;
    StackType_t taskStack[TaskStackDepth];

public:
    void execute();

    INA3221Task() : Task("INA3221 Functions") {}

    void createTask() {
        xTaskCreateStatic(vClassTask<INA3221Task>, this->TaskName,
                          INA3221Task::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                          this->taskStack, &(this->taskBuffer));
    }
};

inline etl::optional<INA3221Task> INA3221Task;