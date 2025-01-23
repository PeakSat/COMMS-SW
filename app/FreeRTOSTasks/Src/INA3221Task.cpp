#include "Logger.hpp"
#include "INA3221Task.hpp"

#define NEW_EQM

void ina3221Task::display(const Channel channel, const bool displayShuntVoltage, const bool displayBusVoltage,
                          const bool displayCurrent, const bool displayPower) {
    etl::string<7> channelString;
    switch (channel) {
        case Channel::RF_UHF:
            channelString.assign("RF_UHF");
            break;
        default:
            channelString.assign("ERROR");
    }

    auto channelIndex = static_cast<std::underlying_type_t<Channel>>(channel);
    if (displayShuntVoltage) {
        auto shuntVoltage = std::get<0>(channelMeasurement.value())[channelIndex];
        LOG_INFO << "Channel shunt Voltage\t" << channelString.data() << ": " << shuntVoltage.value() << " uV ";
    }
    if (displayBusVoltage) {
        auto busVoltage = std::get<1>(channelMeasurement.value())[channelIndex];
        LOG_INFO << "Channel bus Voltage\t" << channelString.data() << ": " << (busVoltage.value()) / 1000 << " mV ";
    }
    if (displayCurrent) {
        auto current = std::get<2>(channelMeasurement.value())[channelIndex];
        LOG_INFO << "Channel shunt current\t" << channelString.data() << ": " << current.value() / 1000 << " mA ";
    }
    if (displayPower) {
        auto power = std::get<3>(channelMeasurement.value())[channelIndex];
        LOG_INFO << "Channel power\t\t" << channelString.data() << ": " << power.value() << " mW";
    }
}

void ina3221Task::execute() {
    INA3221::Error error = INA3221::Error::NO_ERRORS;
#ifdef NEW_EQM
    INA3221::INA3221 ina = INA3221::INA3221(hi2c4, INA3221::INA3221Config(), error);
#else
    INA3221::INA3221 ina = INA3221::INA3221(hi2c1, INA3221::INA3221Config(), error);
#endif
    ina.setup();
    Logger::format.precision(Precision);
    while (true) {
        channelMeasurement = ina.getMeasurement();
        if (not channelMeasurement.has_value())
            LOG_ERROR << "Could not get current measurements!";
        else {
            display(Channel::RF_UHF, true, true, true, true);
        }
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
