#pragma once

#include "Task.hpp"
#include "main.h"
#include "TMP117.hpp"
#include "etl/optional.h"

extern I2C_HandleTypeDef hi2c1;

class TemperatureSensorsTask : public Task {
public:
    void execute();

    TemperatureSensorsTask() : Task("External Temperature Sensors") {}

    void createTask() {
        this->taskHandle = xTaskCreateStatic(vClassTask<TemperatureSensorsTask>, this->TaskName,
                                             TemperatureSensorsTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                             this->taskStack, &(this->taskBuffer));
    }

private:
    const static inline uint16_t DelayMs = 500;
    const static inline uint16_t TaskStackDepth = 2000;
    const static inline uint8_t LoggerPrecision = 2;
    const static uint8_t MaxErrorStringSize = 25;
    const static uint8_t MaxSensorNameSize = 16;
    StackType_t taskStack[TaskStackDepth];

    /** A method that maps an error to a string.
     * @param error The type of error
     * @return      The corresponding string of error.Error strings should be smaller than maxErrorStringSize.
     */
    static etl::string<MaxErrorStringSize> errorString(TMP117::Error error);

    /** A method that maps an I2C address to a descriptive sensor name.
     * @param slaveAddress The I2C address
     * @return             The name of the sensor that uses slaveAddress.It must be smaller than maxSensorNameSize.
     */
    static etl::string<MaxSensorNameSize> sensorName(TMP117::I2CAddress slaveAddress);

    /** This struct holds settings for the sensors. For polling, continuous mode is used
     * (the sensors periodically produce new values). Alert settings are irrelevant for now,
     * since the alert pin was not used.
     */
    TMP117::Config config = {
        TMP117::Continuous,
        static_cast<uint8_t>(3),
        TMP117::Samples8,
        0,
        true,
        false,
        true};

    struct sensor {
        TMP117::TMP117 sensorObject;
        etl::string<MaxSensorNameSize> sensorName;
    };


    etl::array<sensor, 3> sensors = {
        sensor{TMP117::TMP117(hi2c1, TMP117::I2CAddress::Address3, config),
               sensorName(TMP117::Address3)},
        sensor{TMP117::TMP117(hi2c1, TMP117::I2CAddress::Address1, config),
               sensorName(TMP117::I2CAddress::Address1)},
        sensor{TMP117::TMP117(hi2c1, TMP117::I2CAddress::Address4, config),
               sensorName(TMP117::I2CAddress::Address4)},
    };
};

inline etl::optional<TemperatureSensorsTask> tmp117Task;
