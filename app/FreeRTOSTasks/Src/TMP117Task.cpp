#include "Logger.hpp"
#include "TMP117Task.hpp"
#include "eMMC.hpp"
#include "PlatformParameters.hpp"
#include "ParameterService.hpp"


etl::string<TemperatureSensorsTask::MaxErrorStringSize> TemperatureSensorsTask::errorString(TMP117::Error error) {
    switch (error) {
        case TMP117::NoErrors:
            return "NoErrors";
        case TMP117::Timeout:
            return "Timeout";
        case TMP117::InvalidEEPROM:
            return "InvalidEEPROM";
        case TMP117::TemperatureHigh:
            return "TemperatureHigh";
        case TMP117::TemperatureLow:
            return "TemperatureLow";
        case TMP117::NoDataReady:
            return "NoDataReady";
        case TMP117::InvalidCalibrationOffset:
            return "InvalidCalibrationOffset";
        default:
            break;
    }

    return "UnknownError";
}

etl::string<TemperatureSensorsTask::MaxSensorNameSize> TemperatureSensorsTask::sensorName(TMP117::I2CAddress slaveAddress) {
    switch (slaveAddress) {
        case TMP117::I2CAddress::Address3:
            return "PCB";
        case TMP117::I2CAddress::Address1:
            return "PA";
        case TMP117::I2CAddress::Address4:
            return "GNSS";
        default:
            break;
    }

    return "UNKNOWN ADDRESS";
}

void TemperatureSensorsTask::execute() {
    // for temporarily holding a temperature and an error
    etl::pair<TMP117::Error, float> temperature;
    vTaskDelay(100);
    while (true) {
        Logger::format.precision(LoggerPrecision);
        for (auto s: sensors) {
            temperature = s.sensorObject.getTemperature(true);
            if (temperature.first == TMP117::Error::NoErrors) {
                // LOG_DEBUG << "Temperature at " << s.sensorName.data() << ": " << temperature.second;
                if (s.sensorName == "PCB") {
                    COMMSParameters::commsPCBTemperature.setValue(temperature.second);
                }
                else if (s.sensorName == "PA") {
                    COMMSParameters::commsUHFBandPATemperature.setValue(temperature.second);
                }
                else if (s.sensorName == "GNSS") {
                    COMMSParameters::commsGNSSTemperature.setValue(temperature.second);
                }
            } else {
                LOG_ERROR << "Could not get temperature at " << s.sensorName.data() << ". Error: "
                          << errorString(temperature.first).data();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(DelayMs));
    }
}
