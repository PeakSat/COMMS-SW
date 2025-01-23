#pragma once

#include <optional>
#include <cstdint>
#include "main.h"
#include "etl/utility.h"
#include <optional>

namespace TMP117 {

    /**
    The device I2C addresses (Page 21 TMP117 manual)
*/
    enum I2CAddress {
        Address1 = 0x90, /// Connected to Ground
        Address2 = 0x92, /// Connected to VCC
        Address3 = 0x94, /// Connected to SDA
        Address4 = 0x96  /// Connected to SCL
    };

    /**
    Register addresses
*/
    enum RegisterAddress {
        TemperatureRegister = 0x00,
        ConfigurationRegister = 0x01,
        TemperatureHighLimit = 0x02,
        TemperatureLowLimit = 0x03,
        EEPROMUnlock = 0x04,
        EEPROM1 = 0x05,
        EEPROM2 = 0x06,
        EEPROM3 = 0x08,
        TemperatureOffset = 0x07,
        IDRegister = 0x0F
    };

    enum Error {
        NoErrors = 0x0,
        Timeout = 0x1,
        InvalidEEPROM = 0x2,
        TemperatureHigh = 0x3,
        TemperatureLow = 0x4,
        NoDataReady = 0x5,
        InvalidCalibrationOffset = 0x6,
    };

    enum ConversionMode {
        Continuous = 0x0,
        Shutdown = 0x1,
        ContinuousRed = 0x2,
        OneShot = 0x3,
    };

    enum Averaging {
        NoAveraging = 0x0,
        Samples8 = 0x1,
        Samples32 = 0x2,
        Samples64 = 0x3
    };

    struct Config {
        ConversionMode conversionMode;
        uint8_t cycleTime;
        Averaging averaging;
        float temperatureOffset;
        /**
         * 0: Alert Mode
         * 1: Therm Mode
         */
        bool thermalAlert;
        /**
         * 0: Active High
         * 1: Active Low
         */
        bool polarityAlert;
        /**
         * 0: ALERT is used as data ready flag status
         * 1: ALERT is used as alert flag status
         */
        bool drAlert;
    };

    class TMP117 {
    private:
        static constexpr uint16_t MaxTimeoutDelay = 100;
        static constexpr uint8_t TimeoutWait = 10;
        static constexpr uint16_t MaxAbsoluteCalibrationOffset = 256;


        /**
         * HAL I2C handle
         */
        I2C_HandleTypeDef hi2c1;

        /**
         * Slave's address on the I2C bus
         */
        uint8_t i2cSlaveAddress;

        /**
         * TMP117 sensor's ID number
         */
        uint16_t deviceID = 0x0117;

        /**
         * TMP117 sensor's revision number
         */
        uint8_t revisionNumber = 0x00;

    public:
        float temperaturePrecision = 0.0078125;

        /**
         * Holds settings of configuration register
         */
        Config configuration;

        /**
         * Driver for TMP117 sensor
         * @param hi2c1         I2C definition
         * @param address       Address of device in the I2C bus
         * @param config        Used for setting up the configuration register
         */
        TMP117(I2C_HandleTypeDef& hi2c1, I2CAddress address, const Config& config) : hi2c1(hi2c1), i2cSlaveAddress(address), configuration(std::move(config)) {
            configure();
        };

        /**
         * Sets the sensor's offset for calibration offset.
         * @return calibration temperature in Celsius (allowable range [-256, 256] C).
         */
        etl::pair<Error, std::optional<float>> setCalibrationOffset(float calibration);

        /**
         * Sets the sensor's offset for calibration offset.
         * @return calibration temperature in Celsius (allowable range [-256, 256] C).
         */
        etl::pair<Error, float> getCalibrationOffset();


        /**
         * Reads a register with I2C
         *
         * @param deviceAddress     Slave Device Address
         * @param targetRegister    Address of register to be read from
         */
        etl::pair<Error, std::optional<uint16_t>> readRegister(RegisterAddress targetRegister);

        /**
         * Writes a register with I2C
         *
         * @param targetRegister    Address of register to be read from
         * @param value             Data to write to register
         * @return                  Returns possible raised error
         */
        Error writeRegister(RegisterAddress targetRegister, uint16_t value);

        /**
         * Writes to the chip programmable memory.
         * @param eepromRegister         Checks which register to write to. Valid values from 1-3 corresponding to the EEPROM registers
         * @param value     Value to store to target memory
         * @return          Returns possible raised error
         */
        Error writeEEPROM(uint8_t eepromRegister, uint16_t value);

        /**
         * Reads from the chip programmable memory.
         * @param eepromRegister         Checks which register to read from. Valid values from 1-3 corresponding to the EEPROM registers
         * @return          Returns possible raised error
         */
        etl::pair<Error, std::optional<uint16_t>> readEEPROM(uint8_t eepromRegister);

        /**
         * Reprogram configuration register according to the given configuration.
         * Needs to be re-run each time configuration is changed for changes to be applied.
         *
         * @warning Avoid directly reading from the configuration register since alerts will be automatically cleared
         *
         * @return          Returns possible raised error
         */
        Error configure();


        /**
         * Converts the temperature from a 16bit value into a float representing a temperature in the range -+ 256 [C]
         * @param temp  Raw temperature value
         * @return      Formatted value
         */
        float convertTemperature(uint16_t temp);

        /**
         * Gets the temperature of the sensor.
         *
         * @param ignoreAlert   Declares whether for temperatures outside the predefined range an error is raised or not
         * @note This directly reads from the configuration registers and clears any alert flag
         *
         * @warning If set on ALERT mode this will return an error if any error (HIGH/LOW temperature) has been raised in the
         * meantime. To ignore the transients set the temperature to THERM or simply set the ignoreAlert
         *
         * @return Measured temperature in Celsius scale
         */
        etl::pair<Error, float> getTemperature(bool ignoreAlert);

        /**
         * For testing purposes. Returns revision number.
         */

        etl::pair<Error, uint16_t> getRevNumber();

        /**
         * For testing purposes. Returns the device ID.
         */
        etl::pair<Error, uint16_t> getDeviceID();

        /**
         * Handles raised interrupts
         */
        void handleIRQ(void);
    };
} // namespace TMP117