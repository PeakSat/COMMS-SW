#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "queue.h"
#include "etl/array.h"
#include "etl/optional.h"
#include "main.h"

using namespace AT86RF215;

using PacketType = etl::array<uint8_t, MaxPacketLength>;

struct PacketData {
    PacketType packet;
    uint16_t length;
};

class RF_TXTask : public Task {
public:
    RF_TXTask() : Task("RF-TX Task") {}
    void execute();
    PacketData createRandomPacketData(uint16_t length);
    uint8_t transmit_frame_flag, trx_ready = false;
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_TXTask>, this->TaskName,
                                             RF_TXTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 15000;
    constexpr static uint32_t FrequencyUHFTX = 401000;
    AT86RF215::Error error;
    StackType_t taskStack[TaskStackDepth];
};

inline etl::optional<RF_TXTask> rf_txtask;