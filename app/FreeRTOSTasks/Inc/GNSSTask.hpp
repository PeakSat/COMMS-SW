#pragma once
#include <etl/optional.h>
#include "Task.hpp"
#include "queue.h"
#include "GNSSDefinitions.hpp"
#include "GNSS.hpp"
#include "GNSSMessage.hpp"
#include "stm32h7xx_hal.h"
#include <etl/string.h>
#include <etl/expected.h>
#include "minmea.h"

#define printing_frequency 1

extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_uart5_rx;

class GNSSTask : public Task {
public:
    /**
     * stack depth
     */
    static const uint16_t TaskStackDepth = 4000;
    /**
     * array with TaskStackDepth Elements
     */
    StackType_t taskStack[TaskStackDepth];
    /**
     * string for printing of the GNSS data
     */
    etl::string<1024> GNSSMessageString = "";
    /**
     * buffer with that holds the GNSS data
     */
    uint8_t rx_buf[1024] = {0};
    /**
     * pointer that points to the rx_buffer address
     */
    uint8_t* rx_buf_pointer = nullptr;
    /**
     * flag used to change the mode of GNSS each time that gets to the corresponding function
     */
    uint8_t interval_mode_flag = 0;
    /**
     * incoming size in bytes from the GNSS
     */
    uint16_t size_response, size_message = 0;
    /**
     * printing counter to control the number of prints
     */
    uint8_t printing_counter = 0;
    /**
     *  uxItemSize
     */
    unsigned long uxItemSize = sizeof(uint8_t*);
    /**
     * uxQueueLength
     */
    const unsigned long uxQueueLength = 1;
    /**
     * buffer that will holds the pointers
     */
    uint8_t* pucQueueStorage;
    /**
     * will be used to hold the queue's data structure.
     */
    StaticQueue_t* pxQueueBuffer;
    /**
     *  handle for the queue to have a way of separating them
     */
    QueueHandle_t gnssQueueHandleDefault, gnssQueueHandleGNSSResponse;

    uint8_t* sendToQueue;
    uint8_t* sendToQueueResponse;
    uint8_t control = 0;

    /**
     * execute function of the task
     */
    void execute();
    /**
     * prints with a configurable frequency the output of the GNSS
     */
    void printing(uint8_t* buf);

    void parser(uint8_t* buf);
    /**
     *
     * @param gnssMessageToSend
     * @return
     */
    etl::expected<void, ErrorFromGNSS> controlGNSSwithNotify(GNSSMessage gnssMessageToSend);
    /**
     *
     */
    void switchGNSSMode();
    /**
     *
     * @param buf
     * @param size
     */
    static void startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size);
    /**
     *
     */
    void initQueuesToAcceptPointers();

    GNSSTask() : Task("GNSS Logging Task") {}

    void createTask() {
        taskHandle = xTaskCreateStatic(vClassTask<GNSSTask>, this->TaskName,
                                       GNSSTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                       this->taskStack, &(this->taskBuffer));
        initQueuesToAcceptPointers();
    }

private:
};

inline etl::optional<GNSSTask> gnssTask;
