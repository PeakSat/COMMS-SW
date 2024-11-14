#include "INA3221.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace INA3221 {


    uint16_t voltageConversion(int32_t voltage, uint16_t base, uint16_t shift) {
        if (voltage < 0) {
            return (~(std::abs(voltage) / base << shift) + 1 & 0x7FFF) | 0x8000;
        }

        return (voltage / base << shift);
    }

    etl::expected<void, Error> INA3221::i2cWrite(Register address, uint16_t value) {
        etl::array<uint8_t, 3> buffer{to_underlying(address),
                                      static_cast<uint8_t>(value >> 8),
                                      static_cast<uint8_t>(value & 0x00FF)};

        if (HAL_I2C_Master_Transmit(&hi2c, to_underlying(I2CSlaveAddress) << 1, buffer.data(), 3, MaxTimeoutDelay) != HAL_OK) {
            return etl::unexpected<Error>(Error::I2C_FAILURE);
        }

        return {};
    }

    etl::expected<uint16_t, Error> INA3221::i2cRead(Register address) {
        uint8_t buffer[2];
        auto regAddress = to_underlying(address);

        if (HAL_I2C_Master_Transmit(&hi2c, to_underlying(I2CSlaveAddress) << 1, &regAddress, 1, MaxTimeoutDelay) != HAL_OK) {
            return etl::unexpected<Error>(Error::I2C_FAILURE);
        }

        if (HAL_I2C_Master_Receive(&hi2c, to_underlying(I2CSlaveAddress) << 1, buffer, 2, MaxTimeoutDelay) != HAL_OK) {
            return etl::unexpected<Error>(Error::I2C_FAILURE);
        }

        uint16_t received = (static_cast<uint16_t>(buffer[0]) << 8) | static_cast<uint16_t>(buffer[1]);
        return received;
    }

    etl::expected<uint16_t, Error> INA3221::getConfigRegister() {
        return i2cRead(Register::CONFG);
    }

    etl::expected<void, Error>
    INA3221::writeRegisterField(Register registerAddress, uint16_t value, uint16_t mask, uint16_t shift) {
        auto reg = i2cRead(registerAddress);
        if (not reg.has_value()) {
            return etl::unexpected<Error>(reg.error());
        }

        uint16_t val = (reg.value() & ~mask) | (value << shift);
        return i2cWrite(registerAddress, val);
    }

    etl::expected<void, Error> INA3221::changeOperatingMode(OperatingMode operatingMode) {
        config.operatingMode = operatingMode;
        return writeRegisterField(Register::CONFG, to_underlying(operatingMode), 0x7, 0);
    }

    bool INA3221::singleShot() const {
        return config.operatingMode == OperatingMode::SHUNT_VOLTAGE_SS or
               config.operatingMode == OperatingMode::BUS_VOLTAGE_SS or
               config.operatingMode == OperatingMode::SHUNT_BUS_VOLTAGE_SS;
    }

    bool INA3221::busEnabled() const {
        return config.operatingMode == OperatingMode::BUS_VOLTAGE_SS or
               config.operatingMode == OperatingMode::BUS_VOLTAGE_CONT or
               config.operatingMode == OperatingMode::SHUNT_BUS_VOLTAGE_SS or
               config.operatingMode == OperatingMode::SHUNT_BUS_VOLTAGE_CONT;
    }

    bool INA3221::shuntEnabled() const {
        return config.operatingMode == OperatingMode::SHUNT_VOLTAGE_SS or
               config.operatingMode == OperatingMode::SHUNT_VOLTAGE_CONT or
               config.operatingMode == OperatingMode::SHUNT_BUS_VOLTAGE_SS or
               config.operatingMode == OperatingMode::SHUNT_BUS_VOLTAGE_CONT;
    }

    etl::expected<ChannelMeasurement, Error> INA3221::getMeasurement() {
        if (singleShot()) {
            // trigger measurement when on single shot mode
            changeOperatingMode(config.operatingMode);

            // wait for conversion time
            TickType_t ticksToConversion = 0;
            if (shuntEnabled()) {
                ticksToConversion = etl::max(ticksToConversion,
                                             pdMS_TO_TICKS(conversionTimeArray[to_underlying(config.shuntVoltageTime)] / 1000));
            }
            if (busEnabled()) {
                ticksToConversion = etl::max(ticksToConversion,
                                             pdMS_TO_TICKS(conversionTimeArray[to_underlying(config.busVoltageTime)] / 1000));
            }
            vTaskDelay(ticksToConversion);

            // poll conversion bit until ready or conversion time has passed for a second time
            auto initialTickCount = xTaskGetTickCount();
            while (xTaskGetTickCount() - initialTickCount < ticksToConversion) {
                auto maskeValue = i2cRead(Register::MASKE);
                if (not maskeValue.has_value()) {
                    return etl::unexpected<Error>(maskeValue.error());
                }

                if (maskeValue.value() & to_underlying(MaskEnableMasks::CVRF)) { break; }
            }
        }

        VoltageMeasurement shuntMeasurement{etl::nullopt, etl::nullopt, etl::nullopt};
        VoltageMeasurement busMeasurement{etl::nullopt, etl::nullopt, etl::nullopt};
        CurrentMeasurement currentMeasurement{etl::nullopt, etl::nullopt, etl::nullopt};
        PowerMeasurement powerMeasurement{etl::nullopt, etl::nullopt, etl::nullopt};

        if (shuntEnabled()) {
            for (uint8_t channel = 0; channel <= 2; channel++) {
                if (not config.enableChannel[channel]) { continue; }

                auto dummy = getShuntVoltage(channel);
                if (not dummy.has_value()) {
                    return etl::unexpected<Error>(dummy.error());
                }

                shuntMeasurement[channel] = dummy.value();
                currentMeasurement[channel] = shuntMeasurement[channel].value() * ShuntConductance;
            }
        }

        if (busEnabled()) {
            for (uint8_t channel = 0; channel <= 2; channel++) {
                if (not config.enableChannel[channel]) { continue; }

                auto dummy = getBusVoltage(channel);
                if (not dummy.has_value()) {
                    return etl::unexpected<Error>(dummy.error());
                }

                busMeasurement[channel] = dummy.value();
                powerMeasurement[channel] = 1e-9 * busMeasurement[channel].value() * currentMeasurement[channel].value();
            }
        }

        return std::tuple<VoltageMeasurement, VoltageMeasurement, CurrentMeasurement, PowerMeasurement>{shuntMeasurement, busMeasurement, currentMeasurement, powerMeasurement};
    }

    etl::expected<int32_t, Error> INA3221::getShuntVoltage(uint8_t channel) {
        auto registerAddress = static_cast<Register>(to_underlying(Register::CH1SV) + channel * 2);

        auto registerValue = i2cRead(registerAddress);
        if (not registerValue.has_value()) {
            return etl::unexpected<Error>(registerValue.error());
        }

        auto unscaledVolts = static_cast<int16_t>(registerValue.value());
        unscaledVolts >>= 3;
        const int32_t uVolts = static_cast<int32_t>(unscaledVolts) * ShuntVoltBase;

        return uVolts;
    }

    etl::expected<int32_t, Error> INA3221::getBusVoltage(uint8_t channel) {
        auto registerAddress = static_cast<Register>(to_underlying(Register::CH1BV) + channel * 2);

        auto registerValue = i2cRead(registerAddress);
        if (not registerValue.has_value()) {
            return etl::unexpected<Error>(registerValue.error());
        }

        auto unscaledVolts = static_cast<int16_t>(registerValue.value());
        unscaledVolts >>= 3;
        const int32_t uVolts = static_cast<int32_t>(unscaledVolts) * BusVoltBase;

        return uVolts;
    }

    etl::expected<uint16_t, Error> INA3221::getDieID() {
        return i2cRead(Register::DIEID);
    }

    etl::expected<uint16_t, Error> INA3221::getManID() {
        return i2cRead(Register::MFRID);
    }

    etl::expected<void, Error> INA3221::setup() {
        uint16_t mode = (config.enableChannel[0] << 14) | (config.enableChannel[1] << 13) | (config.enableChannel[2] << 12) |
                        (to_underlying(config.averagingMode) << 9) | (to_underlying(config.busVoltageTime) << 6) |
                        (to_underlying(config.shuntVoltageTime) << 3) | (to_underlying(config.operatingMode));

        auto error = i2cWrite(Register::CONFG, mode);
        if (not error.has_value()) { return error; }

        auto [criticalThreshold1, warningThreshold1] = config.threshold1;

        error = i2cWrite(Register::CH1CA, voltageConversion(criticalThreshold1, ShuntVoltBase, 3));
        if (not error.has_value()) { return error; }

        error = i2cWrite(Register::CH1WA, voltageConversion(warningThreshold1, ShuntVoltBase, 3));

        auto [criticalThreshold2, warningThreshold2] = config.threshold2;

        error = i2cWrite(Register::CH2CA, voltageConversion(criticalThreshold2, ShuntVoltBase, 3));
        if (not error.has_value()) { return error; }

        error = i2cWrite(Register::CH2WA, voltageConversion(warningThreshold2, ShuntVoltBase, 3));
        if (not error.has_value()) { return error; }

        auto [criticalThreshold3, warningThreshold3] = config.threshold3;

        error = i2cWrite(Register::CH3CA, voltageConversion(criticalThreshold3, ShuntVoltBase, 3));
        if (not error.has_value()) { return error; }

        error = i2cWrite(Register::CH3WA, voltageConversion(warningThreshold3, ShuntVoltBase, 3));
        if (not error.has_value()) { return error; }

        error = i2cWrite(Register::SHVLL, voltageConversion(config.shuntVoltageSumLimit, ShuntVoltBase, 1));
        if (not error.has_value()) { return error; }

        error = i2cWrite(Register::PWRVU, voltageConversion(config.powerValidUpper, BusVoltBase, 1));
        if (not error.has_value()) { return error; }

        error = i2cWrite(Register::PWRVL, voltageConversion(config.powerValidLower, BusVoltBase, 1));
        if (not error.has_value()) { return error; }

        error = i2cWrite(Register::MASKE,
                         (config.summationChannelControl[0] << 14) | (config.summationChannelControl[1] << 13) | (config.summationChannelControl[2] << 12) | (config.enableWarnings << 11) |
                             (config.enableCritical << 10));

        return error;
    }

    etl::expected<void, Error> INA3221::chipReset() {
        return i2cWrite(Register::CONFG, 0xF000);
    }
} // namespace INA3221