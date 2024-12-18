#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "queue.h"
#include "etl/array.h"
#include "etl/optional.h"
#include <etl/expected.h>
#include "main.h"

using namespace AT86RF215;
class RF_RXTask : public Task {
public:
    typedef enum {
        TX,
        RX
    } rfModes;

    constexpr static uint16_t MaxPacketLength = 1024;
    using PacketType = etl::array<uint8_t, MaxPacketLength>;
    RF_RXTask() : Task("Transceiver signal transmission") {}
    void setRFmode(uint8_t mode);
    void execute();
    etl::expected<void, Error> check_transceiver_connection();
    void createTask() {
        xTaskCreateStatic(vClassTask<RF_RXTask>, this->TaskName,
                          RF_RXTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                          this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 15000;
    /// frequency in kHz
    constexpr static uint32_t FrequencyUHFRX = 401500;
    AT86RF215::Error error;
    StackType_t taskStack[TaskStackDepth];
};

inline etl::optional<RF_RXTask> rf_rxtask;