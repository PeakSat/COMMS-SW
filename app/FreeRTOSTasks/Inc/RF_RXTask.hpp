#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "queue.h"
#include "etl/array.h"
#include "etl/optional.h"
#include <etl/expected.h>
#include "main.h"
#include <Frame.hpp>
extern CRC_HandleTypeDef hcrc;

#define MIN_TC_SIZE 11
#define MAX_TC_SIZE 256

using namespace AT86RF215;

struct ParsedPacket {
    uint8_t packet_version_number;
    uint8_t packet_type;
    uint8_t secondary_header_flag;
    uint16_t application_process_ID;
};


class RF_RXTask : public Task {
public:
    RF_RXTask() : Task("RF RX TASK"){}
    void ensureRxMode();
    bool verifyCRC(uint8_t* RX_BUFF, int16_t corrected_received_length);
    static ParsedPacket parsePacket(uint8_t* RX_BUFF);
    [[noreturn]] void execute();
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_RXTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 3,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    constexpr static uint16_t TaskStackDepth = 10000;
    /// Frequency in kHz
    constexpr static uint32_t FrequencyUHFRX = 401000;
    AT86RF215::Error error = NO_ERRORS;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<RF_RXTask> rf_rxtask;