#include "Logger.hpp"
#include "INA3221Task.hpp"
#include "INA3221.hpp"

void INA3221Task::execute() {

    INA3221::INA3221Config config;
    INA3221::Error error;
    INA3221::INA3221 ina = INA3221::INA3221(hi2c1, INA3221::INA3221Config(), error);
    ina.setup();

    while (true) {
        LOG_DEBUG << "Measuring tempreature";

        auto status = ina.getMeasurement();
        if (status.has_value()) {
            INA3221::ChannelMeasurement measurement = status.value();
            INA3221::VoltageMeasurement voltage1 = std::get<0>(measurement);
            INA3221::VoltageMeasurement voltage2 = std::get<1>(measurement);
            INA3221::CurrentMeasurement voltagec = std::get<2>(measurement);
            INA3221::PowerMeasurement power = std::get<3>(measurement);
            __NOP();
        }
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
