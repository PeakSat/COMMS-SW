#include "at86rf215.hpp"
#include "RF_RXTask.hpp"
#include "RF_TXTask.hpp"


namespace AT86RF215 {

    At86rf215 transceiver = At86rf215(&hspi4);
    TransceiverHandler transceiver_handler;

    void At86rf215::spi_write_8(uint16_t address, uint8_t value, Error& err) {
        uint8_t msg[3] = {static_cast<uint8_t>(0x80 | ((address >> 8) & 0x7F)), static_cast<uint8_t>(address & 0xFF), value};
        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_RESET);
        uint8_t hal_error = HAL_SPI_Transmit(hspi, msg, 3, TIMEOUT);

        if (hal_error != HAL_OK) {
            err = Error::FAILED_WRITING_TO_REGISTER;
            return;
        }

        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_SET);
        err = NO_ERRORS;
    }

    uint8_t At86rf215::spi_read_8(uint16_t address, Error& err) {
        uint8_t msg[2] = {static_cast<uint8_t>((address >> 8) & 0x7F), static_cast<uint8_t>(address & 0xFF)};
        uint8_t response[3];
        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_RESET);
        uint8_t hal_error = HAL_SPI_TransmitReceive(hspi, msg, response, 3,
                                                    TIMEOUT);

        if (hal_error != HAL_OK) {
            err = Error::FAILED_READING_FROM_REGISTER;
            return 0;
        }

        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_SET);
        err = Error::NO_ERRORS;

        return response[2];
    }
    void At86rf215::spi_block_write_8(uint16_t address, uint16_t n, uint8_t* value,
                                      Error& err) {
        uint8_t msg[2] = {static_cast<uint8_t>(0x80 | ((address >> 8) & 0x7F)), static_cast<uint8_t>(address & 0xFF)};
        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_RESET);
        uint8_t hal_error = HAL_SPI_Transmit(hspi, msg, 2, TIMEOUT);
        hal_error = HAL_SPI_Transmit(hspi, value, n, TIMEOUT);

        if (hal_error != HAL_OK) {
            err = Error::FAILED_WRITING_TO_REGISTER;
            return;
        }
        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_SET);

        err = Error::NO_ERRORS;
    }

    uint8_t* At86rf215::spi_block_read_8(uint16_t address, uint8_t n,
                                         uint8_t* response, Error& err) {
        uint8_t msg[2] = {static_cast<uint8_t>((address >> 8) & 0x7F), static_cast<uint8_t>(address & 0xFF)};

        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_RESET);
        uint8_t hal_error = HAL_SPI_TransmitReceive(hspi, msg, response, n + 2,
                                                    TIMEOUT);

        if (hal_error != HAL_OK) {
            err = Error::FAILED_READING_FROM_REGISTER;
            return response;
        }

        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_SET);
        err = NO_ERRORS;
        return response + 2;
    }


    State At86rf215::get_state(Transceiver transceiver, Error& err) {
        uint8_t state;
        if (transceiver == RF09) {
            state = spi_read_8(RF09_STATE, err) & 0x07;
        } else if (transceiver == RF24) {
            state = spi_read_8(RF24_STATE, err) & 0x07;
        }

        if (err != NO_ERRORS) {
            return RF_INVALID;
        }
        err = Error::NO_ERRORS;

        if ((state >= 0x02) && (state <= 0x07)) {
            return static_cast<State>(state);
        } else {
            err = Error::UKNOWN_REQUESTED_STATE;
            return State::RF_INVALID;
        }
    }

    void At86rf215::set_state(Transceiver transceiver, State state_cmd,
                              Error& err) {
        uint8_t state = get_state(transceiver, err);
        if (err != NO_ERRORS) {
            return;
        }

        err = Error::NO_ERRORS;

        switch (state_cmd) {
            case RF_TRXOFF:
                break;
            case RF_TXPREP:
                if ((state != RF_TRXOFF) && (state != RF_RX) && (state != RF_TX)) {
                    err = FAILED_CHANGING_STATE;
                    return;
                }
                break;
            case RF_TX:
            case RF_RX:
                if (state != RF_TXPREP) {
                    err = FAILED_CHANGING_STATE;
                    return;
                }
                break;
            case RF_NOP:
                break;
            case RF_RESET:
                break;
            case RF_SLEEP:
                if ((state != RF_TRXOFF) && (state != RF_SLEEP)) {
                    err = FAILED_CHANGING_STATE;
                    return;
                }
                break;
            default:
                err = FAILED_CHANGING_STATE;
                return;
        }

        if (transceiver == RF09) {
            spi_write_8(RF09_CMD, state_cmd, err);
        } else if (transceiver == RF24) {
            spi_write_8(RF24_CMD, state_cmd, err);
        }
    }

    void At86rf215::chip_reset(Error& error) {
        // Chip reset
        spi_write_8(RegisterAddress::RF_RST, 0x07, error);

        // Reset IRQ status registers
        spi_read_8(RegisterAddress::RF09_IRQS, error);
        spi_read_8(RegisterAddress::RF24_IRQS, error);
        spi_read_8(RegisterAddress::BBC0_IRQS, error);
        spi_read_8(RegisterAddress::BBC1_IRQS, error);
        // Restores the current config settings
        setup(error);
    }

    void At86rf215::set_pll_channel_spacing(Transceiver transceiver,
                                            uint8_t spacing, Error& err) {
        RegisterAddress regscs;

        if (transceiver == RF09) {
            regscs = RF09_CS;
        } else if (transceiver == RF24) {
            regscs = RF24_CS;
        }
        spi_write_8(regscs, spacing, err);
    }

    uint8_t At86rf215::get_pll_channel_spacing(Transceiver transceiver,
                                               Error& err) {
        RegisterAddress regscs;

        if (transceiver == RF09) {
            regscs = RF09_CS;
        } else if (transceiver == RF24) {
            regscs = RF24_CS;
        }
        return spi_read_8(regscs, err);
    }

    void At86rf215::set_pll_channel_frequency(Transceiver transceiver,
                                              uint16_t freq, Error& err) {
        RegisterAddress regcf0h;
        RegisterAddress regcf0l;

        if (transceiver == RF09) {
            regcf0h = RF09_CCF0H;
            regcf0l = RF09_CCF0L;
        } else if (transceiver == RF24) {
            regcf0h = RF24_CCF0H;
            regcf0l = RF24_CCF0L;
        }

        spi_write_8(regcf0l, freq & 0x00FF, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        spi_write_8(regcf0h, (freq & 0xFF00) >> 8, err);
    }

    uint16_t At86rf215::get_pll_channel_frequency(Transceiver transceiver,
                                                  Error& err) {
        RegisterAddress regcf0h;
        RegisterAddress regcf0l;

        if (transceiver == RF09) {
            regcf0h = RF09_CCF0H;
            regcf0l = RF09_CCF0L;
        } else if (transceiver == RF24) {
            regcf0h = RF24_CCF0H;
            regcf0l = RF24_CCF0L;
        }

        uint16_t cf0h = spi_read_8(regcf0h, err) & 0xFF;
        if (err != Error::NO_ERRORS) {
            return 0;
        }
        uint16_t cf0l = spi_read_8(regcf0l, err) & 0xFF;

        return (cf0h << 8) | cf0l;
    }

    uint16_t At86rf215::get_pll_channel_number(Transceiver transceiver,
                                               Error& err) {
        RegisterAddress regcnh;
        RegisterAddress regcnl;

        if (transceiver == RF09) {
            regcnh = RF09_CNM;
            regcnl = RF09_CNL;
        } else if (transceiver == RF24) {
            regcnh = RF24_CNM;
            regcnl = RF24_CNL;
        }

        uint16_t cnh = spi_read_8(regcnh, err) & 0x0100;
        if (err != Error::NO_ERRORS) {
            return 0;
        }
        uint16_t cnl = spi_read_8(regcnl, err) & 0x00FF;

        return (cnh << 8) | cnl;
    }

    void At86rf215::set_pll_bw(PLLBandwidth bw, Error& err) {
        uint8_t reg_pll = spi_read_8(RF09_PLL, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        // Clear bits [5:4] and set new value
        reg_pll &= ~(0x3 << 4);                     // Clear bits [5:4] (0x3 << 4 = 0b0011 0000)
        reg_pll |= (static_cast<uint8_t>(bw) << 4); // Set new value for bits [5:4]

        if (err != Error::NO_ERRORS) {
            return;
        }
    }

    PLLBandwidth At86rf215::get_pll_bw(Error& err) {
        uint8_t bw = (spi_read_8(RF09_PLL, err) >> 4) & 0x03;
        if (err != Error::NO_ERRORS) {
            return PLLBandwidth::BWInvalid;
        }
        return static_cast<PLLBandwidth>(bw);
    }

    PLLState At86rf215::get_pll_state(Transceiver transceiver, Error& err) {
        RegisterAddress regpll;

        if (transceiver == RF09) {
            regpll = RF09_PLL;
        } else if (transceiver == RF24) {
            regpll = RF24_PLL;
        }

        uint8_t state = spi_read_8(regpll, err) & 0x01;
        return static_cast<PLLState>(state);
    }

    void At86rf215::configure_pll(Transceiver transceiver, uint16_t freq,
                                  uint8_t channel_number, PLLChannelMode channel_mode, PLLBandwidth bw,
                                  uint8_t channel_spacing, Error& err) {

        if (get_state(transceiver, err) != RF_TRXOFF) {
            err = Error::INVALID_STATE_FOR_OPERATION;
            return;
        }

        if ((transceiver == Transceiver::RF09 && channel_mode == PLLChannelMode::FineResolution2443) || (transceiver == Transceiver::RF24 && channel_mode == PLLChannelMode::FineResolution450) || (transceiver == Transceiver::RF24 && channel_mode == PLLChannelMode::FineResolution900)) {
            err = Error::INVALID_TRANSCEIVER_FREQ;
            return;
        }

        uint32_t n_chan = (static_cast<uint32_t>(freq) << 8) | (channel_number & 0xFF);

        if ((((channel_mode == PLLChannelMode::FineResolution450) || (channel_mode == PLLChannelMode::FineResolution900)) && (n_chan < 126030 || n_chan > 1340967)) || ((channel_mode == PLLChannelMode::FineResolution2443) && (n_chan < 85700 || n_chan > 296172))) {
            err = Error::INVALID_TRANSCEIVER_FREQ;
            return;
        }
        /// RFn_CS
        set_pll_channel_spacing(transceiver, channel_spacing, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        /// RFn_CCF0L, RFn_CCF0H
        set_pll_channel_frequency(transceiver, freq, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        RegisterAddress reg_cnm;
        RegisterAddress reg_cnl;
        /// RFn_CNL
        if (transceiver == RF09) {
            reg_cnm = RF09_CNM;
            reg_cnl = RF09_CNL;
        } else if (transceiver == RF24) {
            reg_cnm = RF24_CNM;
            reg_cnl = RF24_CNL;
        }

        spi_write_8(reg_cnl, channel_number, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        // Modify only bits [7:6] of reg_cnm (the CNH is not needed for RF09)
        uint8_t reg_value_cm = spi_read_8(reg_cnm, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        /// RFn_CNM
        // Clear bits [7:6] and set the new value
        reg_value_cm &= ~(0x3 << 6);                               // Clear bits [7:6] (0x3 << 6 = 0b1100 0000)
        reg_value_cm |= (static_cast<uint8_t>(channel_mode) << 6); // Set new value for bits [7:6]

        // Write back the updated value
        spi_write_8(reg_cnm, reg_value_cm, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        /// RFn_PLL
        set_pll_bw(bw, err);
    }

    DevicePartNumber At86rf215::get_part_number(Error& err) {
        uint8_t dpn = spi_read_8(RegisterAddress::RF_PN, err);
        if (err != Error::NO_ERRORS) {
            return DevicePartNumber::AT86RF215_INVALID;
        }

        if ((dpn >= 0x34) && (dpn <= 0x36)) {
            return static_cast<DevicePartNumber>(dpn);
        } else {
            err = Error::UKNOWN_PART_NUMBER;
            return DevicePartNumber::AT86RF215_INVALID;
        }
    }

    uint8_t At86rf215::get_version_number(Error& err) {
        uint8_t vn = spi_read_8(RegisterAddress::RF_VN, err);
        if (err != Error::NO_ERRORS) {
            return 0;
        }
        return vn;
    }

    uint8_t At86rf215::get_pll_frequency(Transceiver transceiver, Error& err) {
        RegisterAddress regpll;

        if (transceiver == RF09) {
            regpll = RF09_PLLCF;
        } else if (transceiver == RF24) {
            regpll = RF24_PLLCF;
        }

        uint8_t freq = spi_read_8(regpll, err) & 0x3F;
        if (err != Error::NO_ERRORS) {
            return 0;
        }
        return freq;
    }

    void At86rf215::set_tcxo_trimming(CrystalTrim trim, Error& err) {
        uint8_t trgxcov = spi_read_8(RF_XOC, err) & 0x1F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(RF_XOC, (trgxcov & 0x10) | (static_cast<uint8_t>(trim) & 0x0F),
                    err);
    }

    CrystalTrim At86rf215::read_tcxo_trimming(Error& err) {
        CrystalTrim regxoc =
            static_cast<CrystalTrim>(spi_read_8(RF_XOC, err) & 0x1F);
        if (err != Error::NO_ERRORS) {
            return CrystalTrim::TRIM_INV;
        }
        return regxoc;
    }

    void At86rf215::set_tcxo_fast_start_up_enable(bool fast_start_up, Error& err) {
        uint8_t trgxcov = spi_read_8(RF_XOC, err) & 0x1F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(RF_XOC, (trgxcov & 0x0F) | (fast_start_up << 4), err);
    }

    bool At86rf215::read_tcxo_fast_start_up_enable(Error& err) {
        bool fast_start_up =
            static_cast<bool>((spi_read_8(RF_XOC, err) & 0x10) >> 4);
        return fast_start_up;
    }

    PowerAmplifierRampTime At86rf215::get_pa_ramp_up_time(Transceiver transceiver,
                                                          Error& err) {
        RegisterAddress regtxcutc;

        if (transceiver == RF09) {
            regtxcutc = RF09_TXCUTC;
        } else if (transceiver == RF24) {
            regtxcutc = RF24_TXCUTC;
        }

        uint8_t ramp = spi_read_8(regtxcutc, err) & 0xC0 >> 6;
        return static_cast<PowerAmplifierRampTime>(ramp);
    }

    TransmitterCutOffFrequency At86rf215::get_cutoff_freq(Transceiver transceiver,
                                                          Error& err) {
        RegisterAddress regtxcutc;

        if (transceiver == RF09) {
            regtxcutc = RF09_TXCUTC;
        } else if (transceiver == RF24) {
            regtxcutc = RF24_TXCUTC;
        }

        uint8_t cutoff = spi_read_8(regtxcutc, err) & 0x0F;
        return static_cast<TransmitterCutOffFrequency>(cutoff);
    }


    TxRelativeCutoffFrequency At86rf215::get_relative_cutoff_freq(
        Transceiver transceiver, Error& err) {
        RegisterAddress regtxdfe;

        if (transceiver == RF09) {
            regtxdfe = RF09_TXDFE;
        } else if (transceiver == RF24) {
            regtxdfe = RF24_TXDFE;
        }

        uint8_t dfe = (spi_read_8(regtxdfe, err) & 0x30) >> 5;
        return static_cast<TxRelativeCutoffFrequency>(dfe);
    }


    bool At86rf215::get_direct_modulation(Transceiver transceiver, Error& err) {
        RegisterAddress regtxdfe;

        if (transceiver == RF09) {
            regtxdfe = RF09_TXDFE;
        } else if (transceiver == RF24) {
            regtxdfe = RF24_TXDFE;
        }

        return (spi_read_8(regtxdfe, err) & 0x10) >> 4;
    }


    ReceiverSampleRate At86rf215::get_sample_rate(Transceiver transceiver,
                                                  Error& err) {
        RegisterAddress regtxdfe;

        if (transceiver == RF09) {
            regtxdfe = RF09_TXDFE;
        } else if (transceiver == RF24) {
            regtxdfe = RF24_TXDFE;
        }

        return static_cast<ReceiverSampleRate>(spi_read_8(regtxdfe, err) & 0x1F);
    }

    PowerAmplifierCurrentControl At86rf215::get_pa_dc_current(
        Transceiver transceiver, Error& err) {
        RegisterAddress regpac;

        if (transceiver == RF09) {
            regpac = RF09_PAC;
        } else if (transceiver == RF24) {
            regpac = RF24_PAC;
        }

        uint8_t txpa = spi_read_8(regpac, err) & 0x60 >> 5;
        return static_cast<PowerAmplifierCurrentControl>(txpa);
    }


    bool At86rf215::get_lna_bypassed(Transceiver transceiver, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t lna_bypass = spi_read_8(regaux, err) & 0x80;
        if (err != Error::NO_ERRORS)
            return 0;
        return lna_bypass >> 7;
    }

    AutomaticGainControlMAP At86rf215::get_agcmap(Transceiver transceiver,
                                                  Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t agcmap = spi_read_8(regaux, err) & 0x60;
        if (err != Error::NO_ERRORS)
            return AutomaticGainControlMAP::AGC_INVALID;
        return static_cast<AutomaticGainControlMAP>(agcmap >> 5);
    }


    AutomaticVoltageExternal At86rf215::get_external_analog_voltage(
        Transceiver transceiver, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t agcmap = spi_read_8(regaux, err) & 0x10;
        if (err != Error::NO_ERRORS)
            return AutomaticVoltageExternal::INVALID;
        return static_cast<AutomaticVoltageExternal>(agcmap >> 4);
    }

    bool At86rf215::get_analog_voltage_settled_status(Transceiver transceiver,
                                                      Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t avs = spi_read_8(regaux, err) & 0x04;
        if (err != Error::NO_ERRORS)
            return 0;
        return avs >> 2;
    }
    PowerAmplifierVoltageControl At86rf215::get_analog_power_amplifier_voltage(
        Transceiver transceiver, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t pavc = spi_read_8(regaux, err) & 0x03;
        if (err != Error::NO_ERRORS)
            return PowerAmplifierVoltageControl::PAVC_INVALID;
        return static_cast<PowerAmplifierVoltageControl>(pavc);
    }


    void At86rf215::set_ed_average_detection(Transceiver transceiver, uint8_t df,
                                             EnergyDetectionTimeBasis dtb, Error& err) {
        RegisterAddress regedd;

        if (transceiver == RF09) {
            regedd = RF09_EDD;
        } else if (transceiver == RF24) {
            regedd = RF24_EDD;
        }

        uint8_t reg = ((df & 0xCF) << 2) | (static_cast<uint8_t>(dtb) & 0x3);
        spi_write_8(regedd, reg, err);
    }

    uint8_t At86rf215::get_ed_average_detection(Transceiver transceiver,
                                                Error& err) {
        RegisterAddress regedd;

        if (transceiver == RF09) {
            regedd = RF09_EDD;
        } else if (transceiver == RF24) {
            regedd = RF24_EDD;
        }

        uint8_t reg = spi_read_8(regedd, err);
        uint8_t df = (reg & 0xFC) >> 2;
        uint8_t dtb = (reg * 0x3);
        return df * dtb;
    }

    int8_t At86rf215::get_rssi(Transceiver transceiver,
                               Error& err) {
        RegisterAddress regrssi;

        if (transceiver == RF09) {
            regrssi = RF09_RSSI;
        } else if (transceiver == RF24) {
            regrssi = RF24_RSSI;
        }

        int8_t reg = static_cast<int8_t>(spi_read_8(regrssi, err));

        if (err != Error::NO_ERRORS) {
            return 127;
        }
        if (reg > 4) {
            err = Error::INVALID_RSSI_MEASUREMENT;
            return 127;
        }
        return reg;
    }

    // TODO: Upon reaching RX state
    // wait 8μs + RXDFE.SR + Tu
    // read rssi
    // determine whether received signal strength is acceptable

    void At86rf215::transmitBasebandPacketsTx(Transceiver transceiver,
                                              uint8_t* packet, uint16_t length, Error& err) {
        if (tx_ongoing || rx_ongoing) {
            err = ONGOING_TRANSMISSION_RECEPTION;
            return;
        }

        set_state(transceiver, RF_TRXOFF, err);
        if (err != NO_ERRORS) {
            return;
        }

        RegisterAddress regtxflh;
        RegisterAddress regtxfll;
        RegisterAddress regfbtxs;

        if (transceiver == RF09) {
            regtxflh = BBC0_TXFLH;
            regtxfll = BBC0_TXFLL;
            regfbtxs = BBC0_FBTXS;
        } else if (transceiver == RF24) {
            regtxflh = BBC1_TXFLH;
            regtxfll = BBC1_TXFLL;
            regfbtxs = BBC1_FBTXS;
        }

        /// write length to register
        spi_write_8(regtxfll, length & 0xFF, err);
        if (err != NO_ERRORS) {
            return;
        }
        spi_write_8(regtxflh, (length >> 8) & 0x07, err);
        if (err != NO_ERRORS) {
            return;
        }

        /// write to tx frame buffer

        spi_block_write_8(regfbtxs, length, packet, err);
        if (err != NO_ERRORS) {
            return;
        }
        set_state(transceiver, RF_TXPREP, err);
        tx_ongoing = true;
    }

    void At86rf215::clear_channel_assessment(Transceiver transceiver, Error& err) {
        if (tx_ongoing or rx_ongoing) {
            err = ONGOING_TRANSMISSION_RECEPTION;
        }
        set_state(transceiver, State::RF_TRXOFF, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // rx_ongoing = true;
        cca_ongoing = true;
        set_state(transceiver, State::RF_TXPREP, err);
    }


    void At86rf215::packetReception(Transceiver transceiver, Error& err) {
        if (err != Error::NO_ERRORS) {
            return;
        }

        RegisterAddress regrxflh;
        RegisterAddress regrxfll;
        RegisterAddress regfbtxs;

        if (transceiver == RF09) {
            regrxflh = BBC0_RXFLH;
            regrxfll = BBC0_RXFLL;
            regfbtxs = BBC0_FBTXS;
        } else if (transceiver == RF24) {
            regrxflh = BBC1_RXFLH;
            regrxfll = BBC1_RXFLL;
            regfbtxs = BBC1_FBTXS;
        }

        // read length
        uint8_t length = (spi_read_8(regrxflh, err) << 8) | static_cast<uint16_t>(spi_read_8(regrxfll, err));
        if (err != Error::NO_ERRORS) {
            return;
        }

        spi_block_read_8(regfbtxs, length, received_packet, err);
    }

    void At86rf215::set_battery_monitor_control(BatteryMonitorHighRange range, BatteryMonitorVoltageThreshold threshold, Error& err) {
        if (err != Error::NO_ERRORS) {
            return;
        }
        set_battery_monitor_high_range(range, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        set_battery_monitor_voltage_threshold(threshold, err);
    }


    BatteryMonitorStatus At86rf215::get_battery_monitor_status(Error& err) {
        uint8_t status = (spi_read_8(RF_BMDVC, err) & 0x20) >> 5;
        return static_cast<BatteryMonitorStatus>(status);
    }

    void At86rf215::set_battery_monitor_high_range(BatteryMonitorHighRange range,
                                                   Error& err) {
        uint8_t bmhr = spi_read_8(RF_BMDVC, err) & 0x2F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(RF_BMDVC, (static_cast<uint8_t>(range) << 4) | bmhr, err);
    }

    uint8_t At86rf215::get_battery_monitor_high_range(Error& err) {
        return (spi_read_8(RF_BMDVC, err) & 0x10) >> 4;
    }

    void At86rf215::set_battery_monitor_voltage_threshold(
        BatteryMonitorVoltageThreshold threshold, Error& err) {
        uint8_t reg_value_bmvt = spi_read_8(RF_BMDVC, err);
        reg_value_bmvt &= ~(0xF);
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(RF_BMDVC, reg_value_bmvt | static_cast<uint8_t>(threshold), err);
    }

    uint8_t At86rf215::get_battery_monitor_voltage_threshold(Error& err) {
        return spi_read_8(RF_BMDVC, err) & 0x0F;
    }

    void At86rf215::set_external_front_end_control(Transceiver transceiver, ExternalFrontEndControl frontEndControl, Error& err) {
        RegisterAddress reg_address;
        if (transceiver == RF09)
            reg_address = RF09_PADFE;
        else if (transceiver == RF24)
            reg_address = RF24_PADFE;
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != Error::NO_ERRORS)
            return;
        // clears the bits [7:6]
        reg_value &= ~(0x3 << 6);
        reg_value |= static_cast<uint8_t>(frontEndControl) << 6;
        spi_write_8(reg_address, reg_value, err);
    }

    void At86rf215::setup_tx_frontend(Transceiver transceiver,
                                      PowerAmplifierRampTime pa_ramp_time, TransmitterCutOffFrequency cutoff,
                                      TxRelativeCutoffFrequency tx_rel_cutoff, Direct_Mod_Enable_FSKDM direct_mod,
                                      TransmitterSampleRate tx_sample_rate,
                                      PowerAmplifierCurrentControl pa_curr_control, uint8_t tx_out_power,
                                      ExternalLNABypass ext_lna_bypass, AutomaticGainControlMAP agc_map,
                                      AutomaticVoltageExternal avg_ext, AnalogVoltageEnable av_enable,
                                      PowerAmplifierVoltageControl pa_vcontrol, ExternalFrontEndControl externalFrontEndControl, Error& err) {
        RegisterAddress regtxcut;
        RegisterAddress regtxdfe;
        RegisterAddress regpac;
        RegisterAddress regauxs;

        uint8_t reg = 0;

        if (transceiver == Transceiver::RF09) {
            regtxcut = RF09_TXCUTC;
            regtxdfe = RF09_TXDFE;
            regpac = RF09_PAC;
            regauxs = RF09_AUXS;
        } else if (transceiver == Transceiver::RF24) {
            regtxcut = RF24_TXCUTC;
            regtxdfe = RF24_TXDFE;
            regpac = RF24_PAC;
            regauxs = RF24_AUXS;
        }
        /// Set RFn_TXCUTC
        reg = (static_cast<uint8_t>(pa_ramp_time) << 6) | static_cast<uint8_t>(cutoff);
        spi_write_8(regtxcut, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        /// Set RFn_TXDFE
        reg = (static_cast<uint8_t>(tx_rel_cutoff) << 5) | static_cast<uint8_t>(direct_mod) << 4 | static_cast<uint8_t>(tx_sample_rate);
        spi_write_8(regtxdfe, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        /// Set RFn_PAC
        reg = (static_cast<uint8_t>(pa_curr_control) << 5) | (tx_out_power & 0x1F);
        spi_write_8(regpac, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        /// Set RFn_AUXS
        reg = (static_cast<uint8_t>(ext_lna_bypass) << 7) | (static_cast<uint8_t>(agc_map) << 5) | (static_cast<uint8_t>(avg_ext) << 4) | (static_cast<uint8_t>(av_enable) << 3) | (static_cast<uint8_t>(pa_vcontrol));
        spi_write_8(regauxs, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        /// Set RFn_PADFE
        set_external_front_end_control(transceiver, externalFrontEndControl, err);
    }

    void At86rf215::setup_iq(ExternalLoopback external_loop,
                             IQOutputCurrent out_cur, IQmodeVoltage common_mode_vol,
                             IQmodeVoltageIEE common_mode_iee, EmbeddedControlTX embedded_tx_start,
                             ChipMode chip_mode, SkewAlignment skew_alignment, Error& err) {
        /// Set RF_IQIFC0
        uint8_t reg;
        reg = (static_cast<uint8_t>(external_loop) << 7) | (static_cast<uint8_t>(out_cur) << 4) | (static_cast<uint8_t>(common_mode_vol) << 2) | (static_cast<uint8_t>(common_mode_iee) << 1) | static_cast<uint8_t>(embedded_tx_start);
        spi_write_8(RF_IQIFC0, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        /// Set RF_IQIFC1
        reg = (static_cast<uint8_t>(chip_mode) << 4) | static_cast<uint8_t>(skew_alignment);
        spi_write_8(RF_IQIFC1, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
    }

    void At86rf215::setup_crystal(bool fast_start_up, CrystalTrim crystal_trim,
                                  Error& err) {
        uint8_t reg = (static_cast<uint8_t>(fast_start_up) << 4) | static_cast<uint8_t>(crystal_trim);
        spi_write_8(RF_XOC, reg, err);
    }

    void At86rf215::setup_rx_energy_detection(Transceiver transceiver,
                                              EnergyDetectionMode energy_mode, uint8_t energy_detect_factor,
                                              EnergyDetectionTimeBasis energy_time_basis, Error& err) {
        uint8_t reg_value;
        RegisterAddress regedc;
        RegisterAddress regedd;

        if (transceiver == Transceiver::RF09) {
            regedc = RF09_EDC;
            regedd = RF09_EDD;
        } else if (transceiver == Transceiver::RF24) {
            regedc = RF24_EDC;
            regedd = RF24_EDD;
        }
        /// Read the current register value
        reg_value = spi_read_8(regedc, err);
        if (err != NO_ERRORS) {
            return;
        }

        /// Set RFn_EDC
        /// Clear bits [1:0] and update them with the new value
        reg_value &= ~0x03;                                    // Clear bits [1:0] (0x03 = 0000 0011)
        reg_value |= static_cast<uint8_t>(energy_mode) & 0x03; // Write new value to bits [1:0]
        spi_write_8(regedc, static_cast<uint8_t>(energy_mode), err);
        if (err != NO_ERRORS) {
            return;
        }
        /// Set RFn_EDD
        reg_value = 0;
        reg_value = (energy_detect_factor << 2) | static_cast<uint8_t>(energy_time_basis);
        spi_write_8(regedd, reg_value, err);
    }

    void At86rf215::setup_rx_frontend(Transceiver transceiver, bool if_inversion,
                                      bool if_shift, ReceiverBandwidth rx_bw,
                                      RxRelativeCutoffFrequency rx_rel_cutoff,
                                      ReceiverSampleRate rx_sample_rate, bool agc_input,
                                      AverageTimeNumberSamples agc_avg_sample, AGCReset agc_reset, AGCFreezeControl agc_freeze_control, AGCEnable agc_enable,
                                      AutomaticGainTarget agc_target, uint8_t gain_control_word, Error& err) {
        if (gain_control_word > 0x23) {
            err = INVALID_AGC_CONTROl_WORD;
            return;
        }

        RegisterAddress regrxbwc;
        RegisterAddress regrxdfe;
        RegisterAddress regagcc;
        RegisterAddress regagcs;

        uint8_t reg = 0;

        if (transceiver == Transceiver::RF09) {
            regrxbwc = RF09_RXBWC;
            regrxdfe = RF09_RXDFE;
            regagcc = RF09_AGCC;
            regagcs = RF09_AGCS;
        } else if (transceiver == Transceiver::RF24) {
            regrxbwc = RF24_RXBWC;
            regrxdfe = RF24_RXDFE;
            regagcc = RF24_AGCC;
            regagcs = RF24_AGCS;
        }

        /// Set RFn_RXBWC
        reg = spi_read_8(regrxbwc, err);
        reg = (reg & 0xC0) | (static_cast<uint8_t>(if_inversion) << 5) |
              (static_cast<uint8_t>(if_shift) << 4) |
              static_cast<uint8_t>(rx_bw);
        spi_write_8(regrxbwc, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        /// Set RFn_RXDFE
        reg = spi_read_8(regrxdfe, err);
        reg = (reg & 0x10) | (static_cast<uint8_t>(rx_rel_cutoff) << 5) | static_cast<uint8_t>(rx_sample_rate);
        spi_write_8(regrxdfe, reg, err);
        if (err != NO_ERRORS) {
            return;
        }

        /// Set RFn_AGGC
        reg = spi_read_8(regagcc, err);
        reg = (reg & (0x1 << 7)) | (static_cast<uint8_t>(agc_input) << 6) | (static_cast<uint8_t>(agc_avg_sample) << 4) | (static_cast<uint8_t>(agc_reset) << 3) | (static_cast<uint8_t>(agc_freeze_control) << 1) | (static_cast<uint8_t>(agc_enable) << 0);
        spi_write_8(regagcc, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        /// Set RFn_AGCS
        if (agc_enable == AGCEnable::agc_disabled)
            reg = (static_cast<uint8_t>(agc_target) << 5) | gain_control_word;
        else {
            // Leave bits [4:0] unchanged, update bits [7:5]
            reg &= 0x1F;                                    // Mask out bits [7:5], keep [4:0] unchanged (0x1F = 00011111) because this value indicated the current receiver gain setting
            reg |= (static_cast<uint8_t>(agc_target) << 5); // Write to bits [7:5]
        }
        spi_write_8(regagcs, reg, err);
    }

    void At86rf215::setup_irq_cfg(bool maskMode, IRQPolarity polarity,
                                  PadDriverStrength padDriverStrength, Error& err) {
        RegisterAddress regcfg = RF_CFG;
        uint8_t reg_value = spi_read_8(regcfg, err);
        if (err != NO_ERRORS) {
            return;
        }
        reg_value &= ~(0xF);
        reg_value |= (maskMode << 3) | (static_cast<uint8_t>(polarity) << 2) | static_cast<uint8_t>(padDriverStrength);
        spi_write_8(regcfg, reg_value, err);
    }

    void At86rf215::setup_phy_baseband(Transceiver transceiver, bool continuousTransmit,
                                       bool frameSeqFilter, bool transmitterAutoFCS,
                                       FrameCheckSequenceType fcsType, bool basebandEnable,
                                       PhysicalLayerType phyType, Error& err) {
        RegisterAddress regphy;

        if (transceiver == Transceiver::RF09) {
            regphy = BBC0_PC;
        } else if (transceiver == Transceiver::RF24) {
            regphy = BBC1_PC;
        }

        spi_write_8(regphy,
                    (continuousTransmit << 7) | (frameSeqFilter << 6) | (transmitterAutoFCS << 4) | (static_cast<uint8_t>(fcsType) << 3) | (basebandEnable << 2) | static_cast<uint8_t>(phyType),
                    err);
    }


    void At86rf215::setup_irq_mask(Transceiver transceiver, bool iqIfSynchronizationFailure, bool transceiverError,
                                   bool batteryLow, bool energyDetectionCompletion, bool transceiverReady, bool wakeup,
                                   bool frameBufferLevelIndication, bool agcRelease, bool agcHold,
                                   bool transmitterFrameEnd, bool receiverExtendedMatch, bool receiverAddressMatch,
                                   bool receiverFrameEnd, bool receiverFrameStart, Error& err) {
        RegisterAddress regbbc;
        RegisterAddress regrf;

        if (transceiver == Transceiver::RF09) {
            regbbc = BBC0_IRQM;
            regrf = RF09_IRQM;
        } else if (transceiver == Transceiver::RF24) {
            regbbc = BBC1_IRQM;
            regrf = RF24_IRQM;
        }

        spi_write_8(regrf, iqIfSynchronizationFailure << 5 | transceiverError << 4 | batteryLow << 3 | energyDetectionCompletion << 2 | transceiverReady << 1 | wakeup, err);

        spi_write_8(regbbc,
                    frameBufferLevelIndication << 7 | agcRelease << 6 | agcHold << 5 | transmitterFrameEnd << 4 | receiverExtendedMatch << 3 | receiverAddressMatch << 2 | receiverFrameEnd << 1 | receiverFrameStart, err);
    }

    void At86rf215::setup(Error& err) {
        /// Check state of RF09 core
        State state = get_state(RF09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// We have access to all registers only if we are in the state TRXOFF
        if (state != RF_TRXOFF) {
            err = INVALID_STATE_FOR_OPERATION;
            return;
        }
        /// Check state of RF24 core - we only proceed with the set-up if both cores are in the TRXOFF state to avoid setting half the registers
        state = get_state(RF24, err);
        if (err != NO_ERRORS) {
            return;
        }
        if (state != RF_TRXOFF) {
            err = INVALID_STATE_FOR_OPERATION;
            return;
        }

        /// Set IRQ masks
        setup_irq_mask(Transceiver::RF09, radioInterruptsConfig.iqIfSynchronizationFailure09, radioInterruptsConfig.transceiverError09,
                       radioInterruptsConfig.batteryLow09, radioInterruptsConfig.energyDetectionCompletion09, radioInterruptsConfig.transceiverReady09,
                       radioInterruptsConfig.wakeup09, interruptsConfig.frameBufferLevelIndication09, interruptsConfig.agcRelease09,
                       interruptsConfig.agcHold09, interruptsConfig.transmitterFrameEnd09, interruptsConfig.receiverExtendedMatch09,
                       interruptsConfig.receiverAddressMatch09, interruptsConfig.receiverFrameEnd09, interruptsConfig.receiverFrameStart09, err);

        setup_irq_mask(Transceiver::RF24, radioInterruptsConfig.iqIfSynchronizationFailure24, radioInterruptsConfig.transceiverError24,
                       radioInterruptsConfig.batteryLow24, radioInterruptsConfig.energyDetectionCompletion24, radioInterruptsConfig.transceiverReady24,
                       radioInterruptsConfig.wakeup24, interruptsConfig.frameBufferLevelIndication24, interruptsConfig.agcRelease24,
                       interruptsConfig.agcHold24, interruptsConfig.transmitterFrameEnd24, interruptsConfig.receiverExtendedMatch24,
                       interruptsConfig.receiverAddressMatch24, interruptsConfig.receiverFrameEnd24, interruptsConfig.receiverFrameStart24, err);

        /// Set IRQ pin
        setup_irq_cfg(generalConfig.irqMaskMode, generalConfig.irqPolarity,
                      generalConfig.padDriverStrength, err);

        /// Set PLL
        configure_pll(Transceiver::RF09, freqSynthesizerConfig.channelCenterFrequency09,
                      freqSynthesizerConfig.channelNumber09, freqSynthesizerConfig.channelMode09,
                      freqSynthesizerConfig.loopBandwidth09, freqSynthesizerConfig.channelSpacing09, err);
        if (err != NO_ERRORS) {
            return;
        }

        /// Setup Physical Layer for Baseband Cores
        setup_phy_baseband(Transceiver::RF09, basebandCoreConfig.continuousTransmit09,
                           basebandCoreConfig.frameCheckSequenceFilterEn09, basebandCoreConfig.transmitterAutoFrameCheckSequence09,
                           basebandCoreConfig.frameCheckSequenceType09, basebandCoreConfig.baseBandEnable09,
                           basebandCoreConfig.physicalLayerType09, err);
        if (err != NO_ERRORS) {
            return;
        }
        setup_phy_baseband(Transceiver::RF24, basebandCoreConfig.continuousTransmit24,
                           basebandCoreConfig.frameCheckSequenceFilterEn24, basebandCoreConfig.transmitterAutoFrameCheckSequence24,
                           basebandCoreConfig.frameCheckSequenceType24, basebandCoreConfig.baseBandEnable24,
                           basebandCoreConfig.physicalLayerType24, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// BBCn_FSKC0
        set_bbc_fskc0_config(RF09, basebandCoreConfig.bandwidth_time_09, basebandCoreConfig.midxs_09, basebandCoreConfig.midx_09, basebandCoreConfig.mord_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// BBCn_FSKC1
        set_bbc_fskc1_config(RF09, basebandCoreConfig.freq_inv_09, basebandCoreConfig.sr_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// BBCn_FSKC2
        set_bbc_fskc2_config(RF09, basebandCoreConfig.preamble_detection_09, basebandCoreConfig.receiver_override_09, basebandCoreConfig.receiver_preamble_timeout_09, basebandCoreConfig.mode_switch_en_09, basebandCoreConfig.preamble_inversion_09, basebandCoreConfig.fec_scheme_09, basebandCoreConfig.interleaving_enable_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// BBCn_FSKC3
        set_bbc_fskc3_config(RF09, basebandCoreConfig.sfdt_09, basebandCoreConfig.prdt_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// BBC_FSKC4
        set_bbc_fskc4_config(RF09, basebandCoreConfig.sfdQuantization_09, basebandCoreConfig.sfd32_09, basebandCoreConfig.rawModeReversalBit_09, basebandCoreConfig.csfd1_09, basebandCoreConfig.csfd0_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// BBCn_FSKPHRTX
        set_bbc_fskphrtx(RF09, basebandCoreConfig.sfdUsed_09, basebandCoreConfig.dataWhitening_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// BBCn_FSKDM
        set_bbc_fskdm(RF09, basebandCoreConfig.fskPreamphasisEnable_09, basebandCoreConfig.directModEnableFskdm_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// Set TX front-end
        setup_tx_frontend(Transceiver::RF09, txConfig.powerAmplifierRampTime09,
                          txConfig.transmitterCutOffFrequency09,
                          txConfig.txRelativeCutoffFrequency09, txConfig.directModulation09,
                          txConfig.transceiverSampleRate09,
                          txConfig.powerAmplifierCurrentControl09, txConfig.txOutPower09,
                          externalFrontEndConfig.externalLNABypass09, externalFrontEndConfig.automaticGainControlMAP09,
                          externalFrontEndConfig.automaticVoltageExternal09, externalFrontEndConfig.analogVoltageEnable09,
                          externalFrontEndConfig.powerAmplifierVoltageControl09, externalFrontEndConfig.externalFrontEnd_09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// Set up RX front-end
        setup_rx_frontend(Transceiver::RF09, rxConfig.ifInversion09, rxConfig.ifShift09,
                          rxConfig.receiverBandwidth09, rxConfig.rxRelativeCutoffFrequency09,
                          rxConfig.receiverSampleRate09, rxConfig.agcInput09,
                          rxConfig.averageTimeNumberSamples09, rxConfig.agcReset_09, rxConfig.agcFreezeControl_09, rxConfig.agcEnabled09,
                          rxConfig.automaticGainTarget09, rxConfig.gainControlWord09, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// Set up IQ interface
        setup_iq(iqInterfaceConfig.externalLoopback, iqInterfaceConfig.iqOutputCurrent,
                 iqInterfaceConfig.iqmodeVoltage, iqInterfaceConfig.iqmodeVoltageIEE,
                 iqInterfaceConfig.embeddedControlTX, iqInterfaceConfig.chipMode, iqInterfaceConfig.skewAlignment,
                 err);
        if (err != NO_ERRORS) {
            return;
        }

        /// Set up energy detection
        /// RFn_EDC, RFn_EDD
        setup_rx_energy_detection(Transceiver::RF09, rxConfig.energyDetectionMode09,
                                  rxConfig.energyDetectDurationFactor09, rxConfig.energyDetectionBasis09, err);
        if (err != NO_ERRORS) {
            return;
        }

        /// Set up battery
        /// RF_BMDVC
        set_battery_monitor_control(generalConfig.batteryMonitorHighRange, generalConfig.batteryMonitorVoltage, err);
        if (err != NO_ERRORS) {
            return;
        }

        /// Set up crystal oscillator
        /// RF_XOC
        setup_crystal(generalConfig.fastStartUp, generalConfig.crystalTrim, err);
    }

    uint8_t At86rf215::get_irq(Transceiver transceiver, Error& err) {
        if (transceiver == RF09) {
            return spi_read_8(RF09_IRQS, err);
        } else if (transceiver == RF24) {
            return spi_read_8(RF24_IRQS, err);
        }
        return 0;
    }

    etl::expected<void, Error> At86rf215::check_transceiver_connection(Error& err) {
        DevicePartNumber dpn = transceiver.get_part_number(err);
        if (err == NO_ERRORS && dpn == DevicePartNumber::AT86RF215) {
            return {}; /// success
        } else
            return etl::unexpected<Error>(err);
    }

    void At86rf215::set_bbc_fskc0_config(Transceiver transceiver,
                                         Bandwidth_time_product bt, Mod_index_scale midxs, Mod_index midx, FSK_mod_order mord,
                                         Error& err) {
        /// Define the appropriate register for BBCn_FSKC0 based on the transceiver
        RegisterAddress reg_address;
        if (transceiver == RF09) {
            reg_address = BBC0_FSKC0; // Replace with actual RF09 register address
        } else if (transceiver == RF24) {
            reg_address = BBC1_FSKC0; // Replace with actual RF24 register address
        } else {
            return;
        }

        /// Read the current register value and mask out the fields to preserve other bits
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != NO_ERRORS) {
            return; // Return early if SPI read fails
        }
        reg_value &= 0x00; // Clear the bits that will be set explicitly

        /// Clear existing values in BT, MIDXS, MIDX, and MORD
        reg_value |= ((static_cast<uint8_t>(bt) & 0x03) << 6);    // BT: Bits [7:6]
        reg_value |= ((static_cast<uint8_t>(midxs) & 0x03) << 4); // MIDXS: Bits [5:4]
        reg_value |= ((static_cast<uint8_t>(midx) & 0x07) << 1);  // MIDX: Bits [3:1]
        reg_value |= (static_cast<uint8_t>(mord) & 0x01);         // MORD: Bit [0]

        /// Write the updated value back to the register
        spi_write_8(reg_address, reg_value, err);
    }
    void At86rf215::set_bbc_fskc1_config(Transceiver transceiver,
                                         Freq_Inversion freq_inv, MR_FSK_symbol_rate sr,
                                         Error& err) {
        // Define the appropriate register for BBCn_FSKC1 based on the transceiver
        RegisterAddress reg_address;
        if (transceiver == RF09) {
            reg_address = BBC0_FSKC1; // Replace with actual RF09 register address
        } else if (transceiver == RF24) {
            reg_address = BBC1_FSKC1; // Replace with actual RF24 register address
        } else {
            return;
        }
        /// Read the current register value and mask out the fields to preserve other bits
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != NO_ERRORS) {
            return; // Return early if SPI read fails
        }
        // clear all bits except bit 4 (counting from 4)
        reg_value &= (0x1 << 4);
        reg_value |= (static_cast<uint8_t>(freq_inv) & 0x1) << 5;
        reg_value |= (static_cast<uint8_t>(sr) & 0xF);
        /// Write the updated value back to the register
        spi_write_8(reg_address, reg_value, err);
    }
    void At86rf215::set_bbc_fskc2_config(Transceiver transceiver, Preamble_Detection preamble_det,
                                         Receiver_Override rec_override,
                                         Receiver_Preamble_Timeout rec_preamble_timeout,
                                         Mode_Switch_Enable mode_switch_en,
                                         Preamble_Inversion preamble_inversion,
                                         FEC_Scheme fec_scheme,
                                         Interleaving_Enable interleaving_enable, Error& err) {
        /// Define the appropriate register for BBCn_FSKC2 based on the transceiver
        RegisterAddress reg_address;
        if (transceiver == RF09) {
            reg_address = BBC0_FSKC2; // Replace with actual RF09 register address
        } else if (transceiver == RF24) {
            reg_address = BBC1_FSKC2; // Replace with actual RF24 register address
        } else {
            return;
        }
        /// Read the current register value and mask out the fields to preserve other bits
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != NO_ERRORS) {
            return; // Return early if SPI read fails
        }
        /// Update the register value with provided configurations
        reg_value &= 0x00; // Clear the bits that will be set explicitly
        /// Bit 7: PDTM - Preamble Detection Mode
        reg_value |= (static_cast<uint8_t>(preamble_det) & 0x1) << 7;
        /// Bits 6-5: RXO - Receiver Override
        reg_value |= (static_cast<uint8_t>(rec_override) & 0x3) << 5;
        /// Bit 4: RXPTO - Receiver Preamble Time Out
        reg_value |= (static_cast<uint8_t>(rec_preamble_timeout) & 0x1) << 4;
        /// Bit 3: MSE - Mode Switch Enable
        reg_value |= (static_cast<uint8_t>(mode_switch_en) & 0x1) << 3;
        /// Bit 2: PRI - Preamble Inversion
        reg_value |= (static_cast<uint8_t>(preamble_inversion) & 0x1) << 2;
        /// Bit 1: FECS - FEC Scheme
        reg_value |= (static_cast<uint8_t>(fec_scheme) & 0x1) << 1;
        /// Bit 0: FECIE - Interleaving Enable
        reg_value |= (static_cast<uint8_t>(interleaving_enable) & 0x1) << 0;
        /// Write the updated value back to the register
        spi_write_8(reg_address, reg_value, err);
    }

    void At86rf215::set_bbc_fskc3_config(Transceiver transceiver, SFD_Detection_Threshold sfdDetectionThreshold, Preamble_Detection_Threshold preambleDetectionThreshold, Error& err) {
        /// Define the appropriate register for BBCn_FSKC2 based on the transceiver
        RegisterAddress reg_address;
        if (transceiver == RF09) {
            reg_address = BBC0_FSKC3; // Replace with actual RF09 register address
        } else if (transceiver == RF24) {
            reg_address = BBC1_FSKC3; // Replace with actual RF24 register address
        } else {
            return;
        }
        /// Read the current register value and mask out the fields to preserve other bits
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != NO_ERRORS) {
            return; // Return early if SPI read fails
        }
        /// Update the register value with provided configurations
        reg_value &= 0x00; // Clear the bits that will be set explicitly
        reg_value |= (static_cast<uint8_t>(sfdDetectionThreshold) & 0xF) << 4;
        reg_value |= (static_cast<uint8_t>(preambleDetectionThreshold) & 0xF) << 0;
        /// Write the updated value back to the register
        spi_write_8(reg_address, reg_value, err);
    }

    void At86rf215::set_bbc_fskc4_config(Transceiver transceiver,
                                         SFD_Quantization sfd_quantization,
                                         SFD_32 sfd_32,
                                         Raw_Mode_Reversal_Bit raw_mode_reversal,
                                         CSFD1 csfd1,
                                         CSFD0 csfd0,
                                         Error& err) {
        /// Define the appropriate register address for BBCn_FSKC4 based on the transceiver
        RegisterAddress reg_address;
        if (transceiver == RF09) {
            reg_address = BBC0_FSKC4; // Replace with the actual RF09 register address
        } else if (transceiver == RF24) {
            reg_address = BBC1_FSKC4; // Replace with the actual RF24 register address
        } else {
            return; // Return early if the transceiver is invalid
        }

        /// Read the current register value and mask out the fields to preserve other bits
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != NO_ERRORS) {
            return; // Return early if SPI read fails
        }
        /// clear all bits except bit 7
        reg_value &= (0x01) << 7;
        reg_value |= (static_cast<uint8_t>(sfd_quantization) & 0x1) << 6;
        reg_value |= (static_cast<uint8_t>(sfd_32) & 0x1) << 5;
        reg_value |= (static_cast<uint8_t>(raw_mode_reversal) & 0x1) << 4;
        reg_value |= (static_cast<uint8_t>(csfd1) & 0x3) << 2;
        reg_value |= (static_cast<uint8_t>(csfd0) & 0x3) << 0;
        // Write the updated value back to the register
        spi_write_8(reg_address, reg_value, err);
    }
    void At86rf215::set_bbc_fskphrtx(Transceiver transceiver, SFD_Used sfdUsed, Data_Whitening dataWhitening, Error& err) {
        /// Define the appropriate register address for BBC0_FSKPHRTX based on the transceiver
        RegisterAddress reg_address;
        if (transceiver == RF09) {
            reg_address = BBC0_FSKPHRTX; // Replace with the actual RF09 register address
        } else if (transceiver == RF24) {
            reg_address = BBC1_FSKPHRTX; // Replace with the actual RF24 register address
        } else {
            return; // Return early if the transceiver is invalid
        }

        /// Read the current register value and mask out the fields to preserve other bits
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != NO_ERRORS) {
            return; // Return early if SPI read fails
        }
        /// clear the bits to be updated
        /// 0000 1000 | 0000 0100 = 0000 1100 -> 1111 0011 -> reg_value = reg_value & 1111 0011
        reg_value &= ~((0x1 << 3) | (0x1 << 2));
        reg_value |= (static_cast<uint8_t>(sfdUsed) & 0x1) << 3;
        reg_value |= (static_cast<uint8_t>(dataWhitening) & 0x1) << 2;
        spi_write_8(reg_address, reg_value, err);
    }

    void At86rf215::set_bbc_fskdm(Transceiver transceiver, FSK_Preamphasis_Enable fskPreamphasisEnable, Direct_Mod_Enable_FSKDM directModEnableFskdm, Error& err) {
        /// Define the appropriate register address for BBCn_FSKDM based on the transceiver
        RegisterAddress reg_address;
        if (transceiver == RF09) {
            reg_address = BBC0_FSKDM;
        } else if (transceiver == RF24) {
            reg_address = BBC1_FSKDM;
        } else {
            return;
        }

        /// Read the current register value and mask out the fields to preserve other bits
        uint8_t reg_value = spi_read_8(reg_address, err);
        if (err != NO_ERRORS) {
            return;
        }
        /// clear bits [1:0]
        reg_value &= ~((0x01 << 1) | (0x01 << 0));
        reg_value |= (static_cast<uint8_t>(fskPreamphasisEnable) & 0x1) << 1;
        reg_value |= (static_cast<uint8_t>(directModEnableFskdm) & 0x1) << 0;
        spi_write_8(reg_address, reg_value, err);
    }

    etl::expected<uint16_t, Error> At86rf215::get_received_length(Transceiver transceiver, Error& err) {
        RegisterAddress reg_address_low;
        RegisterAddress reg_address_high;

        /// Determine the appropriate register addresses based on the transceiver
        if (transceiver == RF09) {
            reg_address_low = BBC0_RXFLL;
            reg_address_high = BBC0_RXFLH;
        } else {
            reg_address_low = BBC1_RXFLL;
            reg_address_high = BBC1_RXFLH;
        }
        uint8_t low_length_byte = spi_read_8(reg_address_low, err);
        if (err != NO_ERRORS) {
            return etl::unexpected<Error>(err);
        }

        /// Read the high-length byte
        uint8_t high_length_byte = spi_read_8(reg_address_high, err);
        if (err != NO_ERRORS) {
            return etl::unexpected<Error>(err); // Return the error
        }
        /// Combine the bytes to form the received length
        uint16_t received_length = (static_cast<uint16_t>(high_length_byte) << 8) | low_length_byte;
        return received_length;
    }

    void At86rf215::print_state(Transceiver transceiver, Error& err) {
        State rf_state = get_state(transceiver, err);
        switch (rf_state) {
            case RF_NOP:
                LOG_DEBUG << "STATE: NOP";
                break;
            case RF_SLEEP:
                LOG_DEBUG << "STATE: SLEEP";
                break;
            case RF_TRXOFF:
                LOG_DEBUG << "STATE: TRXOFF";
                break;
            case RF_TX:
                LOG_DEBUG << "STATE: TX";
                break;
            case RF_RX:
                LOG_DEBUG << "STATE: RX";
                break;
            case RF_TRANSITION:
                LOG_DEBUG << "STATE: TRANSITION";
                break;
            case RF_RESET:
                LOG_DEBUG << "STATE: RESET";
                break;
            case RF_INVALID:
                LOG_DEBUG << "STATE: INVALID";
                break;
            case RF_TXPREP:
                LOG_DEBUG << "STATE: TXPREP";
                break;
            default:
                LOG_ERROR << "UNDEFINED";
                break;
        }
    }

void At86rf215::print_error(Error& err) {
    if (err == NO_ERRORS)
        return;
    switch (err) {
        case FAILED_WRITING_TO_REGISTER:
            LOG_ERROR << "FAILED_WRITING_TO_REGISTER";
            break;

        case FAILED_READING_FROM_REGISTER:
            LOG_ERROR << "FAILED_READING_FROM_REGISTER";
            break;

        case FAILED_CHANGING_STATE:
            LOG_ERROR << "FAILED_CHANGING_STATE";
            break;

        case UKNOWN_REQUESTED_STATE:
            LOG_ERROR << "UNKNOWN_REQUESTED_STATE";
            break;

        case UKNOWN_PART_NUMBER:
            LOG_ERROR << "UNKNOWN_PART_NUMBER";
            break;

        case INVALID_TRANSCEIVER_FREQ:
            LOG_ERROR << "INVALID_TRANSCEIVER_FREQ";
            break;

        case INVALID_STATE_FOR_OPERATION:
            LOG_ERROR << "INVALID_STATE_FOR_OPERATION";
            break;

        case INVALID_PLL_CENTER_FREQ:
            LOG_ERROR << "INVALID_PLL_CENTER_FREQ";
            break;

        case UKNOWN_DEVICE_PART_NUMBER:
            LOG_ERROR << "UNKNOWN_DEVICE_PART_NUMBER";
            break;

        case INVALID_RSSI_MEASUREMENT:
            LOG_ERROR << "INVALID_RSSI_MEASUREMENT";
            break;

        case INVALID_AGC_CONTROl_WORD:
            LOG_ERROR << "INVALID_AGC_CONTROl_WORD";
            break;

        case ONGOING_TRANSMISSION_RECEPTION:
            LOG_ERROR << "ONGOING_TRANSMISSION_RECEPTION";
            break;

        default:
            LOG_ERROR << "UNHANDLED_ERROR";
            break;
    }
}

    void At86rf215::handle_irq(void) {
        Error err = NO_ERRORS;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        /* Sub 1-GHz Transceiver */
        /// Radio IRQ
        volatile uint8_t irq = spi_read_8(RegisterAddress::RF09_IRQS, err);
        if ((irq & InterruptMask::IFSynchronization) != 0) {
            IFSynchronization_flag = true;
        }
        if ((irq & InterruptMask::TransceiverError) != 0) {
            // Transceiver Error handling
            TransceiverError_flag = true;
        }
        if ((irq & InterruptMask::BatteryLow) != 0) {
            // Low Voltage
            Voltage_Drop = true;
        }
        if ((irq & InterruptMask::EnergyDetectionCompletion) != 0) {
            EnergyDetectionCompletion_flag = true;
            rx_ongoing = false;
            // cca_ongoing = false;
        }
        if ((irq & InterruptMask::TransceiverReady) != 0) {
            TransceiverReady_flag = true;

            if (rx_ongoing) {
                // Switch to TX state once the transceiver is ready to send
                if (get_state(RF09, err) != RF_RX)
                    set_state(RF09, RF_RX, err);
                // if (cca_ongoing) {
                //     spi_write_8(RF09_EDC, 0x1, err);
                // }
            }
            if (tx_ongoing) {
                // Switch to TX state once the transceiver is ready to send
                if (get_state(RF09, err) != RF_TX)
                    set_state(RF09, RF_TX, err);
            }
        }
        if ((irq & InterruptMask::Wakeup) != 0) {
            Wakeup_flag = true;
            // Wakeup handling
        }
        /// Baseband IRQ
        irq = spi_read_8(RegisterAddress::BBC0_IRQS, err);
        if ((irq & InterruptMask::FrameBufferLevelIndication) != 0) {
            FrameBufferLevelIndication_flag = true;
        }
        if ((irq & InterruptMask::AGCRelease) != 0) {
            // AGC Release handling
            xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyIndexedFromISR(rf_txtask->taskHandle, NOTIFY_INDEX_AGC_RELEASE, AGC_RELEASE, eSetBits, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        if ((irq & InterruptMask::AGCHold) != 0) {
            // AGC Hold handling
            xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyIndexedFromISR(rf_rxtask->taskHandle, NOTIFY_INDEX_AGC, AGC_HOLD, eSetBits, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        if ((irq & InterruptMask::TransmitterFrameEnd) != 0) {
            xHigherPriorityTaskWoken = pdFALSE;
            tx_ongoing = false;
            transceiver_handler.TRANSMIT_FRM_END_CNT++;
            xTaskNotifyIndexedFromISR(rf_txtask->taskHandle, NOTIFY_INDEX_TXFE_TX, TXFE, eSetBits, &xHigherPriorityTaskWoken);
            xTaskNotifyIndexedFromISR(rf_rxtask->taskHandle, NOTIFY_INDEX_TXFE_RX, TXFE, eSetBits, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        if ((irq & InterruptMask::ReceiverExtendMatch) != 0) {
            // Receiver Extended Match handling
            ReceiverExtendMatch_flag = true;
        }
        if ((irq & InterruptMask::ReceiverAddressMatch) != 0) {
            // Receiver Address Match handling
            ReceiverAddressMatch_flag = true;
        }
        if ((irq & InterruptMask::ReceiverFrameEnd) != 0) {
            ReceiverFrameEnd_flag = true;
            if (rx_ongoing)
                rx_ongoing = false;
            xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyIndexedFromISR(rf_rxtask->taskHandle, NOTIFY_INDEX_RXFE_RX, RXFE_RX, eSetBits, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        if ((irq & InterruptMask::ReceiverFrameStart) != 0) {
            rx_ongoing = true;
        }
    }
}