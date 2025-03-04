#pragma once
#include <at86rf215definitions.hpp>
#include "at86rf215config.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <utility>
#include <cstdint>
#include <etl/expected.h>
#include "Logger.hpp"


const uint16_t TIMEOUT = 1000;
typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
extern SPI_HandleTypeDef hspi4;
namespace AT86RF215 {
    class TransceiverHandler {
    public:
        SemaphoreHandle_t resources_mtx = nullptr;
        SemaphoreHandle_t txfeSemaphore_rx = nullptr;
        SemaphoreHandle_t txfeSemaphore_tx = nullptr;
        SemaphoreHandle_t rxfeSemaphore_rx = nullptr;
        SemaphoreHandle_t rxfeSemaphore_tx = nullptr;
        StaticSemaphore_t mtx_buf = {};
        void initialize_semaphore();
    };

    inline void TransceiverHandler::initialize_semaphore() {
        resources_mtx = xSemaphoreCreateMutexStatic(&mtx_buf);
        if (resources_mtx == nullptr) {
            LOG_ERROR << "Failed to create semaphore";
        }
        txfeSemaphore_rx = xSemaphoreCreateBinary();
        txfeSemaphore_tx = xSemaphoreCreateBinary();
        rxfeSemaphore_rx = xSemaphoreCreateBinary();
        rxfeSemaphore_tx = xSemaphoreCreateBinary();
    }

    // rf_state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing)
    enum RF_STATE {
        // rx_ongoing = false, tx_ongoing = false
        READY = 0,
        // rx_ongoing = false, tx_ongoing = true
        TX_ONG = 1,
        // rx_ongoing = true, tx_ongoing = false
        RX_ONG = 2,
        // rx_ongoing = true, tx_ongoing = true
        RX_TX_ONG = 3,
    };


    enum Error {
        NO_ERRORS,
        FAILED_WRITING_TO_REGISTER,
        FAILED_READING_FROM_REGISTER,
        FAILED_CHANGING_STATE,
        UKNOWN_REQUESTED_STATE,
        UKNOWN_PART_NUMBER,
        INVALID_TRANSCEIVER_FREQ,
        INVALID_STATE_FOR_OPERATION,
        INVALID_PLL_CENTER_FREQ,
        UKNOWN_DEVICE_PART_NUMBER,
        INVALID_RSSI_MEASUREMENT,
        INVALID_AGC_CONTROl_WORD,
        ONGOING_TRANSMISSION_RECEPTION,
    };


    inline uint8_t operator&(uint8_t a, InterruptMask b) {
        return a & static_cast<uint8_t>(b);
    }

    class At86rf215 {
    public:
        GeneralConfiguration generalConfig;
        RXConfig rxConfig;
        TXConfig txConfig;
        BasebandCoreConfig basebandCoreConfig;
        FrequencySynthesizer freqSynthesizerConfig;
        ExternalFrontEndConfig externalFrontEndConfig;
        InterruptsConfig interruptsConfig;
        RadioInterruptsConfig radioInterruptsConfig;
        IQInterfaceConfig iqInterfaceConfig;
        /// Flag indicating that a TX procedure is ongoing
        bool tx_ongoing;
        /// Flag indicating that an RX procedure is ongoing
        bool rx_ongoing;
        /// Flag indicating that the Clean Channel Assessment procedure is ongoing
        bool cca_ongoing;
        /// Flag for checking whether the AGC is locked
        bool agc_held;
        SPI_HandleTypeDef* hspi;

        /*
         * Initializer for AT86RF215
         *
         * @param hspi: pointer to the SPI_HandleTypeDef responsible for configuring the SPI.
         *
         */
        // Constructor with general config only
        At86rf215(SPI_HandleTypeDef* hspim)
            : hspi(hspim),
              tx_ongoing(false), rx_ongoing(false), agc_held(false), cca_ongoing(false){}

        void setGeneralConfig(GeneralConfiguration&& GeneralConfig) {
            generalConfig = std::move(GeneralConfig);
        }
        void setRXConfig(RXConfig&& RXConfig) {
            rxConfig = std::move(RXConfig); // Move the new config into rxConfig
        }
        void setTXConfig(TXConfig&& TXConfig) {
            txConfig = std::move(TXConfig); // Move the new config into rxConfig
        }
        void setBaseBandCoreConfig(BasebandCoreConfig&& BasebandCoreConfig) {
            basebandCoreConfig = std::move(BasebandCoreConfig); // Move the new config into rxConfig
        }
        void setFrequencySynthesizerConfig(FrequencySynthesizer&& FrequencySynthesizer) {
            freqSynthesizerConfig = std::move(FrequencySynthesizer); // Move the new config into rxConfig
        }
        void setExternalFrontEndControlConfig(ExternalFrontEndConfig&& ExternalFrontEndConfig) {
            externalFrontEndConfig = std::move(ExternalFrontEndConfig);
        }
        void setInterruptConfig(InterruptsConfig&& InterruptsConfig) {
            interruptsConfig = std::move(InterruptsConfig);
        }
        void setRadioInterruptConfig(RadioInterruptsConfig&& RadioInterruptsConfig) {
            radioInterruptsConfig = std::move(RadioInterruptsConfig);
        }
        void setIQInterfaceConfig(IQInterfaceConfig&& IQInterfaceConfig) {
            iqInterfaceConfig = std::move(IQInterfaceConfig);
        }
        /* Writes a byte to a specified address
         *
         * @param address	Specifies the address to write to
         * @param value		The value to write to the specified address
         * @param err		Pointer to raised error
         */
        void spi_write_8(uint16_t address, uint8_t value, Error& err);

        /* Reads a byte to a specified address
         *
         * @param address	Specifies the address to read from
         * @param err		Pointer to raised error
         * @returns 		Returns the read byte
         */
        uint8_t spi_read_8(uint16_t address, Error& err);

        /* Writes a byte to a specified address
         *
         * @param address	Specifies the address to start writing to
         * @param n			Number of bytes to write
         * @param value		Pointer to array of values to write to address
         * @param err		Pointer to raised error
         */
        void spi_block_write_8(uint16_t address, uint16_t n, uint8_t* value,
                               Error& err);

        /* Reads a byte to a specified address. Assumes that the caller has
         * allocated the expected memory.
         *
         * @param address	Specifies the address to start reading from
         * @param n 		Number of bytes to read.
         * @param response	Returns a pointer to the read bytes
         * @param err		Pointer to raised error
         */
        uint8_t* spi_block_read_8(uint16_t address, uint8_t n, uint8_t* response,
                                  Error& err);
        /*
         * Fetches the current state of the transceiver
         *
         * @param transceiver	Specifies the transceiver used
         * @param err			Pointer to raised error
         */
        State get_state(Transceiver transceiver, Error& err);

        /*
         * Sets the state of the transceiver
         *
         * @param transceiver	Specifies the transceiver used
         * @param state_cmd		Command responsible for changing the state
         * @param err			Pointer to raised error
         */
        void set_state(Transceiver transceiver, State state_cmd, Error& err);

        /*
         * Does chip reset and reads from the interrupt status registers via SPI, resetting them.
         * It also restores the config settings
         * @param error		Pointer to raised error
         */
        void chip_reset(Error& error);

        /*
         * Sets PLL channel spacing (25kHz resolution)
         *
         * @param transceiver	Specifies the transceiver used
         * @param spacing	Configures the channel spacing with a resolution of 25kHz
         * @param err		Pointer to raised error
         */
        void set_pll_channel_spacing(Transceiver transceiver, uint8_t spacing,
                                     Error& err);

        /*
         * Gets PLL channel spacing
         * @param transceiver	Specifies the transceiver used
         * @param err		Pointer to raised error
         */
        uint8_t get_pll_channel_spacing(Transceiver transceiver, Error& err);

        /*
         * Sets the central channel frequency of the PLL
         *
         * @param transceiver	Specifier the transceiver used
         * @param freq 			Central frequency of the PLL
         * @param err			Pointer to raised error
         */
        void set_pll_channel_frequency(Transceiver transceiver, uint16_t freq,
                                       Error& err);

        /*
         * Fetches the central channel frequency of the PLL
         *
         * @param transceiver	Specifier the transceiver used
         * @param err			Pointer to raised error
         */
        uint16_t get_pll_channel_frequency(Transceiver transceiver, Error& err);

        /*
         * Gets the channel number of the PLL
         *
         * @param transceiver	Specifier the transceiver used
         * @param err			Pointer to raised error
         */
        uint16_t get_pll_channel_number(Transceiver transceiver, Error& err);

        /*
         * Gets the loop bandwitdh of the PLL. Options are:
         * 	- Default (0x0)
         * 	- 15% smaller than default (0x1)
         * 	- 15% larger than default (0x2)
         * 	This is only applicable to the RF09 transceiver
         *
         * @param bw	Loopbandwidth of PLL
         * @param err	Pointer to raised error
         */
        void set_pll_bw(PLLBandwidth bw, Error& err);

        /*
         * Gets the loop bandwitdh of the PLL. Options are:
         * 	- Default
         * 	- 15% smaller than default
         * 	- 15% larger than default
         * 	This is only applicable to the RF09 transceiver
         *
         * @param err	Pointer to raised error
         * @returns 	PLL bandwidth
         */
        PLLBandwidth get_pll_bw(Error& err);

        /*
         * Gets the state of the PLL (locked/not locked)
         *
         * @param transceiver		Specify the transceiver used
         * @param err				Pointer to raised error
         */
        PLLState get_pll_state(Transceiver transceiver, Error& err);

        /*
         * Configures the PLL
         *
         * @param transceiver		Specify the transceiver used
         * @param freq 				Central frequency of the PLL
         * @param channel_number	Channel number of the PLL
         * @param channel_mode		Channel mode of the PLL (defines frequency range and stepping)
         * @param bw				Loopbandwith of the PLL
         * @param err				Pointer to raised error
         */
        void configure_pll(Transceiver transceiver, uint16_t freq,
                           uint8_t channel_number, PLLChannelMode channel_mode,
                           PLLBandwidth bw, uint8_t channel_spacing, Error& err);

        /*
         * Gets the part number of the device
         *
         * @param err	Pointer to raised error
         * @returns 	The part number that is one of the following:
         * 					- AT86RF215
         * 					- AT86RF215IQ
         * 					- AT86RF215M
         */
        DevicePartNumber get_part_number(Error& err);

        /*
         * Gets the version number of the device
         *
         * @param err	Pointer to raised error
         */
        uint8_t get_version_number(Error& err);

        /*
         * Gets the PLL frequency
         *
         * @param transceiver	Specify the transceiver used
         * @param err			Pointer to raised error
         * @return 				PLL frequency
         */
        uint8_t get_pll_frequency(Transceiver transceiver, Error& err);

        /*
         * Sets trimming capacitor to match the load of external TCXO (if used), with
         * a precision of 0.3 pF.
         *
         * C_L = 0.5*(C_X + C_TRIM + C_PAR)
         *
         * Where:
         * 	- C_L:		Load of the crystal
         * 	- C_X: 		External capacitor
         * 	- C_TRIM:	Trimming capacitor
         *	- C_PAR:	Parasitic capacitor
         *
         * @param trim	Crystal trimming (0.3 pF precision)
         * @param err	Pointer to raised error
         */
        void set_tcxo_trimming(CrystalTrim trim, Error& err);

        /*
         * Reads trimming capacitor to match the load of external TXCO (if used), with
         * a precision of 0.3 pF.
         *
         * @param err	Pointer to raised error
         */
        CrystalTrim read_tcxo_trimming(Error& err);

        /*
         * Set fast start-up enable option for external crystal oscillator
         * If enabled, it will increase start-up time by 0.8mA while also increasing
         * the start-up time.
         *
         * @oaram fast_start_up		Fast start-up option for TCXO
         * @param er				Pointer to raised error
         */
        void set_tcxo_fast_start_up_enable(bool fast_start_up, Error& err);

        /*
         * Reads fast start-up enable option for external crystal oscillator
         * If enabled, it will increase start-up time by 0.8mA while also increasing
         * the start-up time.
         *
         * @oaram fast_start_up		Fast start-up option for TCXO
         * @param err				Pointer to raised error
         */
        bool read_tcxo_fast_start_up_enable(Error& err);

        /*
         * Set PA ramp-up time in TX chain.
         *
         * Longer ramp-up time requires more power but decreases possible spurious emmissions
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return 					PA ramp-up time
         */
        PowerAmplifierRampTime get_pa_ramp_up_time(Transceiver transceiver,
                                                   Error& err);
        /*
         * Get the low pass cut-off frequency of the filter in the TX chain.
         * For the filter response refer to Figure 6-2, Atmel AT86RF215 datasheet
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return 					Filter cutoff frequency
         */
        TransmitterCutOffFrequency get_cutoff_freq(Transceiver transceiver,
                                                   Error& err);

        /*
         * Get the relative cut-off frequency of the filter in the TX chain.	 *
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return 					Filter cutoff frequency
         */
        TxRelativeCutoffFrequency get_relative_cutoff_freq(Transceiver transceiver,
                                                           Error& err);
        /*
         * Get whether direct modulation is used in the TX chain.
         * Only available for baseband FSK and OQPSK)
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return 					Indicates whether direct modulation is used
         */
        bool get_direct_modulation(Transceiver transceiver, Error& err);

        /*
         * Set the sample rate of the receiver.
         * For exact configuration of the sample_rate refer to AT86RF215 datasheet, Table 6-6
         * or in registers.h*
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return 					Sample rate of receiver
         */
        ReceiverSampleRate get_sample_rate(Transceiver transceiver, Error& err);

        /*
         * Read PA DC current
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return 					PA DC current
         */
        PowerAmplifierCurrentControl get_pa_dc_current(Transceiver transceiver,
                                                       Error& err);

        /*
         * Get whether the external LNA is bypassed
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @retuen					Get whether external LNA is bypassed
         */
        bool get_lna_bypassed(Transceiver transceiver, Error& err);

        /*
         * Shows whether Automatic Gain Control is used for the external LNA.
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return agcmap			AGC gain
         */
        AutomaticGainControlMAP get_agcmap(Transceiver transceiver, Error& err);

        /*
         * Set whether an external analog voltage is supplied to AVDD0 or AVDD1 for the sub-1 GHz
         * and the 2.4 Ghz transceiver respectively
         *
         * @param transceiver		Specifies the transceiver used
         * @oaram avext				Specifies whether external voltage is supplied to AVDD
         * @param err				Pointer to raised error
         * @return					Specifies whether external voltage is supplied to AVDD
         */
        AutomaticVoltageExternal get_external_analog_voltage(
            Transceiver transceiver, Error& err);

        /*
         * Shows whether analog voltage is settled
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return					Specifies whether AV is settled
         */
        bool get_analog_voltage_settled_status(Transceiver transceiver, Error& err);

        /*
         * Fetches supplied voltage of the internal PA
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @return					PA supplied voltage
         */
        PowerAmplifierVoltageControl get_analog_power_amplifier_voltage(
            Transceiver transceiver, Error& err);

        /*
         * Set receiver energy detection average duration given by df*dtb
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         * @param df				Detection factor
         * @param dtb				Detection time scale
         */
        void set_ed_average_detection(Transceiver transceiver, uint8_t df,
                                      EnergyDetectionTimeBasis dtb, Error& err);

        /*
         * Read receiver energy detection average duration given by df*dtb in μs
         *
         * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         */
        uint8_t get_ed_average_detection(Transceiver transceiver, Error& err);
        /*
         * Read RSSI reg
         */
        int8_t get_rssi(Transceiver transceiver, Error& err);

        /*
         * Get transceiver battery monitor status
         *
         * @param err				Pointer to raised error
         * @return status			Battery monitor status
         */
        BatteryMonitorStatus get_battery_monitor_status(Error& err);

        /*
         * Set the threshold of the battery monitoring range (low/high)
         *
         * @param range				Transceiver battery range
         * @param err				Pointer to raised error
         */
        void set_battery_monitor_high_range(BatteryMonitorHighRange range,
                                            Error& err);

        /*
         * Gets the threshold of the battery monitoring range (low/high)
         *
         * @param err				Pointer to raised error
         * @return range			Transceiver battery monitoring range
         */
        uint8_t get_battery_monitor_high_range(Error& err);

        /*
         * Sets voltage threshold for battery monitoring
         *
         * @param threshold			Battery voltage threshold
         * @param err				Pointer to raised error
         */
        void set_battery_monitor_voltage_threshold(BatteryMonitorVoltageThreshold threshold,
                                                   Error& err);
        void set_battery_monitor_control(BatteryMonitorHighRange range, BatteryMonitorVoltageThreshold threshold, Error& err);
        /*
         * Get voltage threshold for battery monitoring
         *
         * @param err				Pointer to raised error
         * @return threshold		Battery voltage threshold
         */
        uint8_t get_battery_monitor_voltage_threshold(Error& err);

        /*
         * Sets up the target registers for setting up the transceiver tx frontend
         *
         * @param transceiver		Specifies the transceiver used
         * @param paramp			TX PA ramp time
         * @param cutoff 			TX filter cut-off frequency
         * @param tx_rel_cutoff     TX relative cut-off frequency
         * @param direct_mod		Specifies whether direct modulation is supported (supported for FSK and OQPSK)
         * @param tx_sample_rate    TX sample rate
         * @param pa_curr_control 	Controls power amplifier current reduction
         * @param transceiver		Specifies the transceiver used
         * @param tx_out_power		Output power of the transmitter (0x00-0x1F in 1dB steps)
         * @param ext_lna_bypass 	Specifies whether external LNA will be bypassed
         * @param agc_map			Controls gain of the gain controler for the external LNA
         * @param av_ext			Disables internal supply voltage
         * @param av_enable			Defines whether voltage regulator is enabled during TRXOFF
         * @param pa_vcontrol		Controls supply voltage of internal PA
         * @param err				Pointer to raised error
         */
        void setup_tx_frontend(Transceiver transceiver,
                               PowerAmplifierRampTime pa_ramp_time,
                               TransmitterCutOffFrequency cutoff,
                               TxRelativeCutoffFrequency tx_rel_cutoff, Direct_Mod_Enable_FSKDM direct_mod,
                               TransmitterSampleRate tx_sample_rate,
                               PowerAmplifierCurrentControl pa_curr_control, uint8_t tx_out_power,
                               ExternalLNABypass ext_lna_bypass, AutomaticGainControlMAP agc_map,
                               AutomaticVoltageExternal avg_ext, AnalogVoltageEnable av_enable,
                               PowerAmplifierVoltageControl pa_vcontrol, ExternalFrontEndControl externalFrontEndControl, Error& err);

        /*
         * Sets up the target registers for setting up the transceiver rx frontend
         *
         * @param transceiver		Specifies the transceiver used
         * @param if_inversion		Defines whether IF inverted signal is used in the receive side
         * @param if_sheft			If true, it shifts the IF frequency by a factor of 1.25
         * @param rx_bw				Specifies the receiver bandwidth
         * @param rx_rel_cutoff		RX filter relative cut-off frequency
         * @param rx_sample_rate	RX sample rate
         * @param agc_input			If true, the filtered front signal is used rather than the signal before the channel filter
         * @param agc_avg_sample	AGC averaging
         * @param agc_enabled 		If set to true AGC is enabled, otherwise, the gain is defined by the agc_gain parameter (AGCS.GCW register)
         * @param agc_target		Sets the target output gain of the AGC
         * @param agc_gcw			If AGC is not enabled, then this register is used to define the maximum gain (valid values 0-23 with 3dB steps)
         * @param err				Pointer to raised error
         */
        void setup_rx_frontend(Transceiver transceiver, bool if_inversion,
                               bool if_shift, ReceiverBandwidth rx_bw,
                               RxRelativeCutoffFrequency rx_rel_cutoff,
                               ReceiverSampleRate rx_sample_rate, bool agc_input,
                               AverageTimeNumberSamples agc_avg_sample, AGCReset agc_reset, AGCFreezeControl agc_freeze_control, AGCEnable agc_enable,
                               AutomaticGainTarget agc_target, uint8_t gain_control_word, Error& err);
        /*
         * Set up IQ interface
         *
         * @param ext_loopback		Defines whether external loopback is enabled (for testing purposes only)
         * @param out_cur			Defines output current
         * @param common_mode_vol	Voltage of I/Q signals
         * @param common_mode_ieee	Whether voltage of I/Q signals is set to 1V2 (IEEE Std 1596-compliant)
         * @param embedded_tx_start	Specifies whether a control bit is automatically transmitted upon start and finish of IQ stream
         * @param chip_mode			Defines what operates out of the baseband core and I/Q IF
         * @param skew_alignment	Specifies the alignment of I/Q data relative to the clock edges of RXCLK
         */
        void setup_iq(ExternalLoopback external_loop, IQOutputCurrent out_cur,
                      IQmodeVoltage common_mode_vol, IQmodeVoltageIEE common_mode_iee,
                      EmbeddedControlTX embedded_tx_start, ChipMode chip_mode,
                      SkewAlignment skew_alignment, Error& err);

        /**
         * Sets up parameters for received energy tracking
         *
         * @param transceiver				Specifies the transceiver used
         * @param energy_mode				Energy detection measurement mode (AUTO/Single/Continuous/Off)
         * @param energy_detect_factor		Duration factor over which the results will be averaged (mult by time base)
         * @param energy_time_basis			Time basis multiplied by the detection factor to determine the averaging window
         * @param err						Pointer to raised error
         */
        void setup_rx_energy_detection(Transceiver transceiver, EnergyDetectionMode energy_mode,
                                       uint8_t energy_detect_factor,
                                       EnergyDetectionTimeBasis energy_time_basis, Error& err);

        /**
         * Sets up internal crystal oscillator
         *
         * @param fast_start_up				Fast start-up option for TCXO (quicker start-up at the expense of current consumption)
         * @param crystal_trim				Controls trim-capacitor to match load capacitance of external oscillator
         * @param err 						Pointer to raised error
         */
        void setup_crystal(bool fast_start_up, CrystalTrim crystal_trim,
                           Error& err);

        /**
         * Sets up IRQ behavior
         *
         * @param maskMode				Defines whether reasons for IRQ call appear in IRQS register
         * @param polarity				Sets up the IRQ pin polarity (active high or low)
         * @param padDriverStrength		Driver strength (mA) of MISO, IRQ and FEA/FEB pins
         * @param err					Pointer to returned error
         */
        void setup_irq_cfg(bool maskMode, IRQPolarity irqPolarity,
                           PadDriverStrength padDriverStrength, Error& err);

        /**
         * Sets up physical baseband
         *
         * @param transceiver			Specifies the transceiver used
         * @param continousTransmit 	Transmission continues for as long as PC.CTX is set
         * @param frameSeqFilter		Successful frame reception IRQ is only triggered if the frame's FCS is valid
         * @param transmitterAutoFCS	Define whether the FCS is inserted automatically to the PSU
         * @param fcsType				16- or 32-bit FCS
         * @param basebandEnable		Sets whether the baseband is enabled (as opposed to the radio mode)
         * @param phyType				Defines the physical layer type
         * @param err					Pointer to returned error
         */
        void setup_phy_baseband(Transceiver transceiver, bool continuousTransmit, bool frameSeqFilter, bool transmitterAutoFCS,
                                FrameCheckSequenceType fcsType, bool basebandEnable, PhysicalLayerType phyType, Error& err);

        void setup_irq_mask(Transceiver transceiver, bool iqIfSynchronizationFailure, bool transceiverError,
                            bool batteryLow, bool energyDetectionCompletion, bool transceiverReady, bool wakeup,
                            bool frameBufferLevelIndication, bool agcRelease, bool agcHold,
                            bool transmitterFrameEnd, bool receiverExtendedMatch, bool receiverAddressMatch,
                            bool receiverFrameEnd, bool receiverFrameStart, Error& err);

        /*
         * Sets up the target registers based on the default configuration. It accesses *all* writable registers and
         * therefore, it requires the transceiver to be in the `TXPREP` state.
         *
         * @param err				Pointer to raised error
         */
        void setup(Error& err);

        /*
         *
         * Returns the IRQ register from the corresponding transceiver
         *
         * @param transceiver		Target transceiver
         * @param err				Pointer to raised error
         */
        uint8_t get_irq(Transceiver transceiver, Error& err);

        void set_bbc_fskc0_config(Transceiver transceiver,
                                  Bandwidth_time_product bt, Mod_index_scale midxs, Mod_index midx, FSK_mod_order mord,
                                  Error& err);
        void set_bbc_fskc1_config(Transceiver transceiver,
                                  Freq_Inversion freq_inv, MR_FSK_symbol_rate sr,
                                  Error& err);
        void set_bbc_fskc2_config(Transceiver transceiver, Preamble_Detection preamble_det,
                                  Receiver_Override rec_override,
                                  Receiver_Preamble_Timeout rec_preamble_timeout,
                                  Mode_Switch_Enable mode_switch_en,
                                  Preamble_Inversion preamble_inversion,
                                  FEC_Scheme fec_sheme,
                                  Interleaving_Enable interleaving_enable, Error& err);
        void set_bbc_fskc3_config(Transceiver transceiver, SFD_Detection_Threshold sfdDetectionThreshold,
                                  Preamble_Detection_Threshold preambleDetectionThreshold,
                                  Error& err);
        void set_bbc_fskc4_config(Transceiver transceiver,
                                  SFD_Quantization sfd_quantization,
                                  SFD_32 sfd_32,
                                  Raw_Mode_Reversal_Bit raw_mode_reversal,
                                  CSFD1 csfd1,
                                  CSFD0 csfd0,
                                  Error& err);
        void set_bbc_fskphrtx(Transceiver transceiver,
                              SFD_Used sfdUsed,
                              Data_Whitening dataWhitening,
                              Error& err);
        void set_bbc_fskdm(Transceiver transceiver,
                           FSK_Preamphasis_Enable fskPreamphasisEnable,
                           Direct_Mod_Enable_FSKDM directModEnableFskdm,
                           Error& err);
        void set_external_front_end_control(Transceiver transceiver,
                                            ExternalFrontEndControl frontEndControl,
                                            Error& err);
        /*
         * This function is called automatically whenever an interrupt is raised. It reads the interrupt status registers
         * and takes action depending on the raised interrupt status.
         */
        void handle_irq();

        /**
         * Measures the received energy in the bandwidth specified
         * @param transceiver       Selected transceiver
         * @param err               Pointer to raised error
         */
        // TODO: Perhaps specify bw here as optional parameter or just control it via the config?
        void clear_channel_assessment(Transceiver transceiver, Error& err);

        /**
         * Begins transmitting operations for Tx packet and automatically sets the `tx_ongoing` flag to inhibit conflicting
         * transmissions. The flag is automatically reset on a frame end interrupt.
         *
         * @param transceiver		Specifies the transceiver used
         * @param packet			Pointer to packet data
         * @param length			Length of packet
         * @param err				Pointer to raised error
         *
         */
        void transmitBasebandPacketsTx(Transceiver transceiver, uint8_t* packet,
                                       uint16_t length, Error& err);

        etl::expected<uint16_t, Error> get_received_length(Transceiver transceiver, Error& err);
        etl::expected<void, Error> check_transceiver_connection(Error& err);
        void packetReception(Transceiver transceiver, Error& err);
        void print_state(Transceiver transceiver, Error& err);
        void print_error(Error& err);
        uint8_t received_packet[2047];
        /// Radio interrupts
        bool IFSynchronization_flag, TransceiverError_flag, EnergyDetectionCompletion_flag, TransceiverReady_flag, Wakeup_flag, Voltage_Drop = false;

        /// Baseband Core Interrupts
        bool FrameBufferLevelIndication_flag, AGCRelease_flag, AGCHold_flag, TransmitterFrameEnd_flag, ReceiverExtendMatch_flag, ReceiverAddressMatch_flag, ReceiverFrameEnd_flag, ReceiverFrameStart_flag = false;

        /**
         * This is automatically called after triggering the packet reception
          * @param transceiver		Specifies the transceiver used
         * @param err				Pointer to raised error
         */
    };

    extern At86rf215 transceiver;
    extern TransceiverHandler transceiver_handler;



} // namespace AT86RF215