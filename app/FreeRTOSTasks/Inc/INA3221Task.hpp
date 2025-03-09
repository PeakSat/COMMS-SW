#pragma once
#include <etl/optional.h>
#include "Task.hpp"
#include "INA3221.hpp"
#include "TaskConfigs.hpp"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c4;


class ina3221Task : public Task {
public:
    /**
     * Functionality of each channel of INA3221 on the comms board
     */
    enum class Channel : uint8_t {
        RF_UHF = 1
    };


    /**
     * Tuple which stores everything the driver returns
     * <0> Array of 3 elements which stores the shunt voltages in uV
     * <1> Array of 3 elements which stores the bus voltages in uV
     * <2> Array of 3 elements which stores the currents in uA
     * <3> Array of 3 elements which stores the consumed powers in mW
     */
    etl::expected<INA3221::ChannelMeasurement, INA3221::Error> channelMeasurement;

    /**
     * Prints current, shunt voltage, bus voltage, power consumption of the input channel
     * @param channel input channel
     * @param displayShuntVoltage enable/disable shunt voltage display
     * @param displayBusVoltage enable/disable bus voltage display
     * @param displayCurrent enable/disable current display
     * @param displayPower enable/disable power consumption display
     */
    void display(Channel channell, bool displayShuntVoltage = true, bool displayBusVoltage = true,
                 bool displayCurrent = true, bool displayPower = true);

    void execute();

    ina3221Task() : Task("Current Sensors") {}

    void createTask() {
        xTaskCreateStatic(vClassTask<ina3221Task>, this->TaskName,
                          INA3221TaskStack, this, INA3221TaskPriority,
                          this->taskStack, &(this->taskBuffer));
    }

private:
    static constexpr uint16_t DelayMs = 4000;
    static constexpr uint8_t Precision = 2;
    StackType_t taskStack[INA3221TaskStack];
};


inline etl::optional<ina3221Task> ina3221Task;