#pragma once
#include <etl/optional.h>
#include "Task.hpp"
#include "queue.h"
#include "GNSSDefinitions.hpp"
#include "GNSS.hpp"
#include "GNSSMessage.hpp"
#include "stm32h7xx_hal.h"
#include <etl/string.h>

#define printing_frequency 5
#define nmea_string_interval 10

extern DMA_HandleTypeDef hdma_uart5_rx;
extern UART_HandleTypeDef huart5;


class GNSSTask : public Task {
public:
    static GNSSReceiver gnssReceiver;

    void execute();
    /**
     * prints with a configurable frequency the output of the GNSS
     */
    void printing();
    /** @code
    * controlGNSS(gnssReceiver.setFactoryDefaults(GNSSDefinitions::DefaultType::Reserved));
    * @endcode
    * @description send appropriate messages to the GNSS by sending them via UART
     *
    * */
    static void controlGNSS(GNSSMessage gnssMessageToSend);
    /**
     *
     * @param nmeaStrings vector that holds the strings you want to configure
     * @param interval specified in seconds
     * @param attributes
     */
    static void changeIntervalofNMEAStrings(etl::vector<etl::string<3>, 10>& nmeaStrings, uint8_t interval, Attributes attributes);
    /**
     *
     * @param nmeaStrings vector that holds the strings you want to configure
     */
    void initializeNMEAStrings(etl::vector<etl::string<3>, 10>& nmeaStrings);
    /**
     * Stack of the task
     */
    const static inline uint16_t TaskStackDepth = 2000;

    StackType_t taskStack[TaskStackDepth];
    /**
     * string for printing of the GNSS data
     */
    etl::string<512> GNSSMessageString = {};
    /**
     * buffer with that holds the GNSS data
     */
    uint8_t incomingMessage[512];
    /**
     * incoming size in bytes from the GNSS
     */
    uint16_t size = 0;
    /**
     * printing counter to control the number of prints
     */
    uint8_t printing_counter = 0;


    /**
     * Queue for incoming messages
     * TO DO, in the future if the processing takes up a lot of time
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

    void createTask() {
        taskHandle = xTaskCreateStatic(vClassTask<GNSSTask>, this->TaskName,
                                       GNSSTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                       this->taskStack, &(this->taskBuffer));
    }

private:
};

inline etl::optional<GNSSTask> gnssTask;
