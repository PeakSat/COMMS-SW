#pragma once
#include "Task.hpp"
#include "task.h"
#include "at86rf215.hpp"
#include "etl/array.h"
#include "etl/optional.h"
#include <Frame.hpp>

struct TX_PACKET_HANDLER {
    uint8_t* pointer_to_data;
    uint16_t data_length;
};

inline TX_PACKET_HANDLER tx_handler;


inline QueueHandle_t TXQueue;
inline StaticQueue_t TXQueueBuffer;
constexpr uint8_t TXQueueItemNum = 20;
constexpr size_t TXItemSize  = sizeof(tx_handler);
inline uint8_t TXQueueStorageArea[TXQueueItemNum * TXItemSize] __attribute__((section(".dtcmram_outgoingTMQueueStorageArea")));
inline uint8_t outgoing_TX_BUFF[1024] __attribute__((section(".dtcmram_tx_buff"), aligned(4)));
inline uint8_t TX_BUF_CAN[1024];
using namespace AT86RF215;


class RF_TXTask : public Task {
public:
    RF_TXTask() : Task("RF-TX Task") {}
    void print_state();
    [[noreturn]]void execute();
    void ensureTxMode();
    void transmitWithWait(uint8_t* tx_buf, uint16_t length, uint16_t wait_ms_for_txfe, AT86RF215::Error& error);
    uint32_t txfe_counter = 0, tx_counter = 0, txfe_not_received, rxfe_received, rxfe_not_received = 0;
    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<RF_TXTask>, this->TaskName,
                                             this->TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }
private:
    constexpr static uint16_t TaskStackDepth = 12000;
    /// Frequency in kHz
    constexpr static uint32_t FrequencyUHFTX = 401000;
    AT86RF215::Error error = NO_ERRORS;
    StackType_t taskStack[TaskStackDepth]{};
};

inline etl::optional<RF_TXTask> rf_txtask;