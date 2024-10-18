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

    const static inline uint16_t TaskStackDepth = 2000;

    StackType_t taskStack[TaskStackDepth];

    /**
* Buffer that holds the data of the DMA
*/
    typedef etl::string<GNSSMessageSize> GNSSMessage;

    uint8_t incomingMessage[512];


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

        // disabling the half buffer interrupt //
        __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
        // disabling the full buffer interrupt //
        __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);

//        HAL_UARTEx_ReceiveToIdle_DMA(&huart5, this->incomingMessage, 512);
    }

    /**
     * Create freeRTOS Task
     */
    void createTask() {
        taskHandle = xTaskCreateStatic(vClassTask < GNSSTask > , this->TaskName,
                                       GNSSTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                       this->taskStack, &(this->taskBuffer));
    }
private:

};

inline etl::optional<GNSSTask> gnssTask;
