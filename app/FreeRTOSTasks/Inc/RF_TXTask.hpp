#pragma once
#include "Task.hpp"
#include "at86rf215.hpp"
#include "etl/optional.h"
#include "TaskConfigs.hpp"

struct TX_PACKET_HANDLER {
    uint8_t buf[1024];
    uint16_t data_length;
};

inline QueueHandle_t TXQueue;
inline StaticQueue_t TXQueueBuffer;
constexpr uint8_t TXQueueItemNum = 4;
constexpr size_t TXItemSize  = sizeof(TX_PACKET_HANDLER);
inline uint8_t TXQueueStorageArea[TXQueueItemNum * TXItemSize] __attribute__((section(".dtcmram_outgoingTMQueueStorageArea")));
inline uint8_t outgoing_TX_BUFF[2048] __attribute__((section(".dtcmram_tx_buff"), aligned(4)));
using namespace AT86RF215;


class RF_TXTask : public Task {
public:
    RF_TXTask() : Task("RF-TX Task") {}
    void print_state();
    [[noreturn]]void execute();
    void ensureTxMode();
    void transmitWithWait(uint8_t* tx_buf, uint16_t length, uint16_t wait_ms_for_txfe, AT86RF215::Error& error, bool enableCrc);
    uint32_t txfe_counter = 0, tx_counter = 0, txfe_not_received, rxfe_received, rxfe_not_received = 0, tx_ong_counter = 0;
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_TXTask>, this->TaskName,
                                             RF_TXTaskStack, this, RF_TXTaskPriority,
                                             this->taskStack, &(this->taskBuffer));
    }
private:
    /// Frequency in kHz
    constexpr static uint32_t FrequencyUHFTX = 401000;
    AT86RF215::Error error = NO_ERRORS;
    StackType_t taskStack[RF_TXTaskStack]{};
};

inline etl::optional<RF_TXTask> rf_txtask;