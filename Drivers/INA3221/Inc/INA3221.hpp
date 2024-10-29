#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>
#include "etl/utility.h"
#include "etl/expected.h"
#include "etl/optional.h"
#include "etl/array.h"
#include "stm32h7xx_hal.h"

extern I2C_HandleTypeDef hi2c4;
extern I2C_HandleTypeDef hi2c1;

namespace INA3221 {


    /**
     * Converts an enumeration member to its underlying type.
     * Equivalent to std::to_underlying (which is not available atm)
     */
    template <typename T>
    constexpr auto to_underlying(T value) {
        return static_cast<std::underlying_type_t<T>>(value);
    }

    /// Used for storing the upper and lower voltage bounds in uV
    using VoltageThreshold = etl::pair<int32_t, int32_t>;

    /// Used for storing the voltage (either shunt or bus) value of the 3 channels in uV
    using VoltageMeasurement = etl::array<etl::optional<int32_t>, 3>;
    /// Used for storing the current value of the 3 channels in uA
    using CurrentMeasurement = etl::array<etl::optional<int32_t>, 3>;
    /// Used for storing the power consumption value of the 3 channels in mW
    using PowerMeasurement = etl::array<etl::optional<float>, 3>;
    /// Used for storing all of the measurements of the 3 channels
    using ChannelMeasurement = std::tuple<VoltageMeasurement, VoltageMeasurement, CurrentMeasurement, PowerMeasurement>;

    /// Device I2C addresses
    enum class I2CAddress : uint16_t {
        /// Connected to GND
        ADDRESS1 = 0x40,
        /// Connected to VS
        ADDRESS2 = 0x41,
        /// Connected to SDA
        ADDRESS3 = 0x42,
        /// Connected to SCL
        ADDRESS4 = 0x43
    };

    /// Register address
    enum class Register : uint8_t {
        /// Configuration
        CONFG = 0x00,
        /// Channel 1 Shunt Voltage
        CH1SV = 0x01,
        /// Chanel 1 Bus Voltage
        CH1BV = 0x02,
        /// Channel 2 Shunt Voltage
        CH2SV = 0x03,
        /// Chanel 2 Bus Voltage
        CH2BV = 0x04,
        /// Channel 3 Shunt Voltage
        CH3SV = 0x05,
        /// Chanel 3 Bus Voltage
        CH3BV = 0x06,
        /// Chanel 1 Critical Alert Limit
        CH1CA = 0x07,
        /// Chanel 1 Warning Alert Limit
        CH1WA = 0x08,
        /// Chanel 2 Critical Alert Limit
        CH2CA = 0x09,
        /// Chanel 2 Warning Alert Limit
        CH2WA = 0x0A,
        /// Chanel 3 Critical Alert Limit
        CH3CA = 0x0B,
        /// Chanel 3 Warning Alert Limit
        CH3WA = 0x0C,
        /// Shunt-Voltage Sum
        SHVOL = 0x0D,
        /// Shunt-Voltage Sum Limit
        SHVLL = 0x0E,
        /// Mask-Enable
        MASKE = 0x0F,
        /// Power-Valid Upper Limit
        PWRVU = 0x10,
        /// Power-Valid Lower Limit
        PWRVL = 0x11,
        /// Manufacturer ID
        MFRID = 0xFE,
        /// Die ID
        DIEID = 0xFF,
    };

    /// Alert level
    enum class Alert : uint8_t {
        CRITICAL = 0,
        WARNING,
        POWER_VALID,
        TIMING_CONTROL
    };

    /// Error status
    enum class Error : uint8_t {
        NO_ERRORS = 0,
        I2C_FAILURE,
        INVALID_STATE,
    };

    /// The number of voltage samples that are averaged together
    enum class AveragingMode : uint16_t {
        AVG_1 = 0,
        AVG_4 = 1,
        AVG_16 = 2,
        AVG_64 = 3,
        AVG_128 = 4,
        AVG_256 = 5,
        AVG_512 = 6,
        AVG_1024 = 7,
    };

    /// Bus voltage conversion time
    enum class ConversionTime : uint16_t {
        /// 140 us
        V_140_MS = 0,
        /// 204 us
        V_204_MS = 1,
        /// 332 us
        V_332_MS = 2,
        /// 588 us
        V_588_MS = 3,
        /// 1.1 ms
        V_1_1_MS = 4,
        /// 2.116 ms
        V_2_116_MS = 5,
        /// 4.156 ms
        V_4_156_MS = 6,
        /// 8.244 ms
        V_8_244_MS = 7,
    };

    /// Array to accommodate conversion Times in us
    constexpr etl::array<uint32_t, 8> conversionTimeArray{140, 204, 332, 588, 1100, 2116, 4156, 8244};

    enum class MaskEnableMasks : uint16_t {
        /// conversion ready flag
        CVRF = 0x0001,
        /// timing control alert flag
        TCF = 0x0002,
        /// power-valid alert flag
        PVF = 0x0004,
        /// warning alert flag, channel 1
        WF1 = 0x0008,
        /// warning alert flag, channel 2
        WF2 = 0x0010,
        /// warning alert flag, channel 3
        WF3 = 0x0020,
        /// summation flag
        SF = 0x0040,
        /// critical alert flag, channel 1
        CF1 = 0x0080,
        /// critical alert flag, channel 2
        CF2 = 0x0100,
        /// critical alert flag, channel 3
        CF3 = 0x0200,
    };

    /**
     * Operating mode of INA3211. The main three modes are:
     *  - Power down: Turns off the current drawn to reduce power consumption. Switching from power-down
     *  mode takes approximately 40 us. I2C communication is still enabled while in this mode
     *  - Single shot: Measurements are taken only whenever this register is written and set to single shot mode
     *  (there's no need to switch the value from a previous different state).
     *  - Continuous: Measurements are constantly taken periodically until the mode is switched to either
     *  single-shot or power down
     *
     *  This register also controls whether this is applies only to shunt voltage, bus voltage
     *  or both.
     */
    enum class OperatingMode : uint16_t {
        POWER_DOWN = 0,             /// Power-down
        SHUNT_VOLTAGE_SS = 1,       /// Shunt Voltage, Single-Shot
        BUS_VOLTAGE_SS = 2,         /// Bus Voltage, Single-Shot
        SHUNT_BUS_VOLTAGE_SS = 3,   /// Shunt & Bus Voltage, Single-Shot
        POWER_DOWN_REND = 4,        /// Power-down
        SHUNT_VOLTAGE_CONT = 5,     /// Shunt Voltage, Continuous
        BUS_VOLTAGE_CONT = 6,       /// Bus Voltage, Continuous
        SHUNT_BUS_VOLTAGE_CONT = 7, /// Shunt & Bus Voltage, Continuous
    };

    /**
     * Config of INA3221. It sets up the necessary values upon powering the device
     */
    struct INA3221Config {

        /// Determines which shunt voltage measurement channels are enabled to fill the Shunt-Voltage Sum register
        etl::array<bool, 3> summationChannelControl{true, true, true};

        /// Warning alerts enabled
        bool enableWarnings = true;

        /// Critical alerts enabled
        bool enableCritical = true;

        /// Determines which of the 3 channels are enabled
        etl::array<bool, 3> enableChannel{true, true, true};

        /// The number of voltage samples that are averaged together
        AveragingMode averagingMode = AveragingMode::AVG_1;

        /**
         * Time of bus voltage measurement conversion.
         * This value should be selected according to the time requirements of the application.
         * Note that it applies to all channels
         */
        ConversionTime busVoltageTime = ConversionTime::V_1_1_MS;

        /**
         * Time of shunt voltage measurement conversion.
         * This value should be selected according to the time requirements of the application.
         * Note that it applies to all channels
         */
        ConversionTime shuntVoltageTime = ConversionTime::V_1_1_MS;

        /**
         * Select mode operation of INNA3211. The main three modes are:
         * - Power down: Turns off the current drawn to reduce power consumption. Switching from power-down
         * mode takes approximately 40 us. I2C communication is still enabled while in this mode
         * - Single shot: Measurements are taken only whenever this register is written and set to single shot mode
         * (there's no need to switch the value from a previous different state).
         * - Continuous: Measurements are constantly taken periodically until the mode is switched to either
         * single-shot or power down
         *
         * This register also controls whether this is applies only to shunt voltage, bus voltage
         * or both.
         */
        OperatingMode operatingMode = OperatingMode::SHUNT_BUS_VOLTAGE_CONT;

        /// Shunt voltage threshold for critical and warning alert for channel1 [uV]
        VoltageThreshold threshold1;

        /// Shunt voltage threshold for critical and warning alert for channel2 [uV]
        VoltageThreshold threshold2;

        /// Shunt voltage threshold for critical and warning alert for channel3 [uV]
        VoltageThreshold threshold3;

        /// Shunt voltage sum limit [uV]
        int32_t shuntVoltageSumLimit;

        /// Upper limit of power-valid [uV]
        int32_t powerValidUpper;

        /// Lower limit of power-valid [uV]
        int32_t powerValidLower;
    };

    class INA3221 {
    public:
        /**
         * Sets the config register as per the passed config
         */
        etl::expected<void, Error> setup();

        /**
         * Resets the chip
         */
        etl::expected<void, Error> chipReset();

        /**
         * Triggers a measurement of bus or shunt voltage for the active channels. Note that the driver halts until we
         * get a valid reading or an alert is raised.
         *
         * If the mode is set to continuous, values will be updated periodically depending on the samples and conversion
         * time set. For single-shot mode, a measurement will be taken only once and the register will need to be
         * re-written in order to get a new measurements
         */
        etl::expected<void, Error> changeOperatingMode(OperatingMode operatingMode);

        /**
         * @return true if the current operating mode has bus measurements enabled, false otherwise
         */
        bool busEnabled() const;

        /**
         * @return true if the current operating mode has shunt measurements enabled, false otherwise
         */
        bool shuntEnabled() const;

        /**
         * @return true if the current operating mode is a single shot mode, false otherwise
         */
        bool singleShot() const;

        /**
         * Get Bus and Shunt voltage measurement in uV, Current measurement in uA, and Power in mW of all channels
         *
         * @return pair of measurement; first is the bus voltage and second the shunt voltage
         */
        etl::expected<ChannelMeasurement, Error> getMeasurement();

        /**
         * Get channel shunt voltage
         *
         * @param channel channel identification number, from 0 to 2 corresponding to channels 1 to 3 respectively
         * @return the bus voltage of the channel in uV
         */
        etl::expected<int32_t, Error> getShuntVoltage(uint8_t channel);

        /**
         * Get channel bus voltage
         *
         * @param channel channel identification number, from 0 to 2 corresponding to channels 1 to 3 respectively
         * @return the bus voltage of the channel in uV
         */
        etl::expected<int32_t, Error> getBusVoltage(uint8_t channel);

        /**
         * Return the value of Die ID register. Testing only.
         */
        etl::expected<uint16_t, Error> getDieID();

        /**
         * Return the value of Man ID register. Testing only.
         */
        etl::expected<uint16_t, Error> getManID();

        /**
         * Return the value of the Config Register.
         */
        etl::expected<uint16_t, Error> getConfigRegister();

        INA3221(I2C_HandleTypeDef& hi2c, INA3221Config config, Error& error) : hi2c(hi2c), config(std::move(config)) {
            auto temporary = setup();
            if (not temporary.has_value()) {
                error = temporary.error();
            }
        };

    private:
        /**
         * HAL I2C Handle
         */
        I2C_HandleTypeDef hi2c;

        /**
         * I2C Bus Slave Address
         */
        static constexpr I2CAddress I2CSlaveAddress = I2CAddress::ADDRESS1;

        /**
         * Inverse of the shunt resistor value in Mhos, namely shunt conductance
         */
        static constexpr uint16_t ShuntConductance = 100;

        /**
         * Shunt Voltage registers base unit in uV
         */
        static constexpr uint16_t ShuntVoltBase = 40;

        /**
         * Bus Voltage registers base unit in uV
         */
        static constexpr uint16_t BusVoltBase = 8000;

        /**
         * Maximum delay until timeout in I2C read/write operation
         */
        static constexpr uint16_t MaxTimeoutDelay = 100;

        /**
         * Writes two bytes in the given register via I2C
         *
         * @param address       Register address
         * @param value         16-bit value to write to
         * @return              Error status
         */
        etl::expected<void, Error> i2cWrite(Register address, uint16_t value);

        /**
         * Reads a given 16-bit register via I2C
         *
         * @param address       Register address
         * @return              read value and error status
         */
        etl::expected<uint16_t, Error> i2cRead(Register address);

        /**
         * Writes to a specific field of the register
         * @param registerAddress       Register registerAddress
         * @param value         Value to write to
         * @param mask          Mask of the value
         * @param shift         Shift bits - determines the register field to write to
         * @return              Error status
         */
        etl::expected<void, Error> writeRegisterField(Register registerAddress, uint16_t value, uint16_t mask, uint16_t shift);

        INA3221Config config;
    };
} // namespace INA3221
