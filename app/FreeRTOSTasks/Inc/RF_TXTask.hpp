#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "etl/array.h"
#include "etl/optional.h"

#define MaxPacketLength 2046


using namespace AT86RF215;

using PacketType = etl::array<uint8_t, MaxPacketLength>;

struct PacketData {
    PacketType packet;
    uint16_t length;
};

class RF_TXTask : public Task {
public:
    RF_TXTask() : Task("RF-TX Task") {}
    void print_state();
    [[noreturn]]void execute();
    void ensureTxMode();
    static PacketData createRandomPacketData(uint16_t length);
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_TXTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }
private:
    constexpr static uint16_t TaskStackDepth = 15000;
    /// Frequency in kHz
    constexpr static uint32_t FrequencyUHFTX = 401000;
    AT86RF215::Error error = NO_ERRORS;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<RF_TXTask> rf_txtask;