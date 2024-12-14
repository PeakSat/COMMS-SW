#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "queue.h"
#include "etl/array.h"
#include "etl/optional.h"
#include "main.h"

using namespace AT86RF215;
extern SPI_HandleTypeDef hspi4;

class RF_RXTask : public Task {
public:
    constexpr static uint16_t MaxPacketLength = 1024;
    using PacketType = etl::array<uint8_t, MaxPacketLength>;
    RF_RXTask() : Task("Transceiver signal transmission") {}
    void execute();
    uint8_t checkTheSPI();
    void createTask() {
        xTaskCreateStatic(vClassTask<RF_RXTask>, this->TaskName,
                          RF_RXTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                          this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t DelayMs = 1;
    constexpr static uint16_t TaskStackDepth = 15000;
    constexpr static uint32_t FrequencyUHFRX = 401000;
    AT86RF215::Error error;
    StackType_t taskStack[TaskStackDepth];
    AT86RF215::AT86RF215Configuration CustomConfig;
};

inline etl::optional<RF_RXTask> rf_rxtask;