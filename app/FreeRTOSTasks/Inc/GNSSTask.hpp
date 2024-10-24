#pragma once

#include <etl/optional.h>
#include "Task.hpp"
#include "queue.h"
#include "GNSSDefinitions.hpp"
#include "GNSS.hpp"
#include "GNSSMessage.hpp"
#include "stm32h7xx_hal.h"
#include <etl/string.h>


extern DMA_HandleTypeDef hdma_uart5_rx;
extern UART_HandleTypeDef huart5;


class GNSSTask : public Task {
public:
    /**
     * @see ParameterStatisticsService
     */
    void execute();

    void printing();

    const static inline uint16_t TaskStackDepth = 2000;

    StackType_t taskStack[TaskStackDepth];
    /**
     * string for printing the GNSS data
     */
    etl::string<512> GNSSMessage = {};
    /**
     * buffer with that holds the GNSS data
     */
    uint8_t incomingMessage[512];
    /**
     * incoming size in bytes from the GNSS
     */
    uint16_t size = 0;
    /**
     * flag that enables the processing of data
     * TO DO: enable notification
     */
    uint8_t gnss_flag = 0;
    /**
     * printing counter to control the number of prints
     */
    uint8_t printing_counter = 0;
    /**
     * Queue for incoming messages
     */
    uint8_t messageQueueStorageArea[GNSSQueueSize * sizeof(GNSSMessage)];
    StaticQueue_t gnssQueue;
    QueueHandle_t gnssQueueHandle;

    GNSSTask() : Task("GNSS Logging Task") {
        gnssQueueHandle = xQueueCreateStatic(GNSSQueueSize, sizeof(GNSSMessage),
                                             messageQueueStorageArea,
                                             &gnssQueue);
        configASSERT(gnssQueueHandle);
    }

    /**
     * Create freeRTOS Task
     */
    void createTask() {
        taskHandle = xTaskCreateStatic(vClassTask<GNSSTask>, this->TaskName,
                                       GNSSTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                       this->taskStack, &(this->taskBuffer));
    }

private:
};

inline etl::optional<GNSSTask> gnssTask;
