#include "at86rf215.hpp"
#include "main.h"
//#include "FreeRTOS.h"
//#include "task.h"
#include "Task.hpp"
#include "RF_TXTask.hpp"
#include "RF_RXTask.hpp"


namespace AT86RF215 {

    AT86RF215::At86rf215 transceiver = At86rf215(&hspi4, AT86RF215Configuration());

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
    int8_t At86rf215::int_spi_read_8(uint16_t address, Error& err) {
        uint8_t msg[2] = {static_cast<uint8_t>((address >> 8) & 0x7F), static_cast<uint8_t>(address & 0xFF)};
        uint8_t response[3];
        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_RESET);
        uint8_t hal_error = HAL_SPI_TransmitReceive(hspi, msg, reinterpret_cast<uint8_t*>(response), 3,
                                                    TIMEOUT);

        if (hal_error != HAL_OK) {
            err = Error::FAILED_READING_FROM_REGISTER;
            return 0;
        }

        HAL_GPIO_WritePin(RF_SPI_SEL_GPIO_Port, RF_SPI_SEL_Pin, GPIO_PIN_SET);
        err = Error::NO_ERRORS;

        return static_cast<int8_t>(response[2]);
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

    void At86rf215::spi_write_16(uint16_t address, uint16_t value, Error& err) {
        uint8_t val[] = {static_cast<uint8_t>((value & 0xFF00) >> 8), static_cast<uint8_t>(value & 0x00FF)};
        // spi_block_write_8(address, 2, val);
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

    uint16_t At86rf215::spi_read_16(uint16_t address, Error& err) {
        uint8_t resp[2];
        uint8_t* val = spi_block_read_8(address, 2, resp, err);
        return (static_cast<uint16_t>(resp[0]) << 8) | resp[1];
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
        uint8_t lbp = spi_read_8(RF09_PLL, err) & 0b11001111;
        if (err != Error::NO_ERRORS) {
            return;
        }
        spi_write_8(RF09_PLL, lbp | (static_cast<uint8_t>(bw) << 4), err);
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
                                  uint16_t channel_number, PLLChannelMode channel_mode, PLLBandwidth bw,
                                  uint8_t channel_spacing, Error& err) {

        if (get_state(transceiver, err) != RF_TRXOFF) {
            err = Error::INVALID_STATE_FOR_OPERATION;
            return;
        }

        if ((transceiver == Transceiver::RF09 && channel_mode == PLLChannelMode::FineResolution2443) || (transceiver == Transceiver::RF24 && channel_mode == PLLChannelMode::FineResolution450) || (transceiver == Transceiver::RF24 && channel_mode == PLLChannelMode::FineResolution900)) {
            err = Error::INVALID_TRANSCEIVER_FREQ;
            return;
        }

        uint32_t nchan = (static_cast<uint32_t>(freq) << 8) | channel_number & 0xFF;

        if ((((channel_mode == PLLChannelMode::FineResolution450) || (channel_mode == PLLChannelMode::FineResolution900)) && (nchan < 126030 || nchan > 1340967)) || ((channel_mode == PLLChannelMode::FineResolution2443) && (nchan < 85700 || nchan > 296172))) {
            err = Error::INVALID_TRANSCEIVER_FREQ;
            return;
        }

        set_pll_channel_frequency(transceiver, freq, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        RegisterAddress regcnh;
        RegisterAddress regcnl;

        if (transceiver == RF09) {
            regcnh = RF09_CNM;
            regcnl = RF09_CNL;
        } else if (transceiver == RF24) {
            regcnh = RF24_CNM;
            regcnl = RF24_CNL;
        }

        spi_write_8(regcnl, channel_number & 0x00FF, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        uint8_t cm = (static_cast<uint8_t>(channel_mode) << 6 | regcnh);
        spi_write_8(regcnh, cm, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        set_pll_channel_spacing(transceiver, channel_spacing, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
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

    void At86rf215::set_pll_frequency(Transceiver transceiver, uint8_t freq,
                                      Error& err) {
        RegisterAddress regpll;

        if (freq > 0x3F) {
            err = Error::INVALID_PLL_CENTER_FREQ;
            return;
        }

        if (transceiver == RF09) {
            regpll = RF09_PLLCF;
        } else if (transceiver == RF24) {
            regpll = RF24_PLLCF;
        }

        spi_write_8(regpll, freq, err);
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

    void At86rf215::set_pa_ramp_up_time(Transceiver transceiver,
                                        PowerAmplifierRampTime pa_ramp_time, Error& err) {
        RegisterAddress regtxcutc;

        if (transceiver == RF09) {
            regtxcutc = RF09_TXCUTC;
        } else if (transceiver == RF24) {
            regtxcutc = RF24_TXCUTC;
        }

        uint8_t txcutc = spi_read_8(regtxcutc, err) & 0x3F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regtxcutc, txcutc | (static_cast<uint8_t>(pa_ramp_time) << 6),
                    err);
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

    void At86rf215::set_cutoff_freq(Transceiver transceiver,
                                    TransmitterCutOffFrequency cutoff, Error& err) {
        RegisterAddress regtxcutf;

        if (transceiver == RF09) {
            regtxcutf = RF09_TXCUTC;
        } else if (transceiver == RF24) {
            regtxcutf = RF24_TXCUTC;
        }

        uint8_t txcutf = spi_read_8(regtxcutf, err) & 0xF0;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regtxcutf, txcutf | static_cast<uint8_t>(cutoff), err);
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

    void At86rf215::set_relative_cutoff_freq(Transceiver transceiver,
                                             TxRelativeCutoffFrequency cutoff, Error& err) {
        RegisterAddress regtxdfe;

        if (transceiver == RF09) {
            regtxdfe = RF09_TXDFE;
        } else if (transceiver == RF24) {
            regtxdfe = RF24_TXDFE;
        }

        uint8_t dfe = spi_read_8(regtxdfe, err) & 0x1F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regtxdfe, dfe | (static_cast<uint8_t>(cutoff) << 5), err);
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

    void At86rf215::set_direct_modulation(Transceiver transceiver, bool dmod,
                                          Error& err) {
        RegisterAddress regtxdfe;
        // TODO: Also set FSKDM.EN and OQPSKC0.DN once implemented
        if (transceiver == RF09) {
            regtxdfe = RF09_TXDFE;
        } else if (transceiver == RF24) {
            regtxdfe = RF24_TXDFE;
        }

        uint8_t dfe = spi_read_8(regtxdfe, err) & 0xEF;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regtxdfe, dfe | (static_cast<uint8_t>(dmod) << 4), err);
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

    void At86rf215::set_sample_rate(Transceiver transceiver,
                                    ReceiverSampleRate sample_rate, Error& err) {
        RegisterAddress regtxdfe;
        if (transceiver == RF09) {
            regtxdfe = RF09_TXDFE;
        } else if (transceiver == RF24) {
            regtxdfe = RF24_TXDFE;
        }

        uint8_t dfe = spi_read_8(regtxdfe, err) & 0xE0;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regtxdfe, dfe | static_cast<uint8_t>(sample_rate), err);
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

    void At86rf215::set_pa_dc_current(Transceiver transceiver,
                                      PowerAmplifierCurrentControl gain, Error& err) {
        RegisterAddress regpac;

        if (transceiver == RF09) {
            regpac = RF09_PAC;
        } else if (transceiver == RF24) {
            regpac = RF24_PAC;
        }

        uint8_t txpa = spi_read_8(regpac, err) & 0x1F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regpac, txpa | (static_cast<uint8_t>(gain) & 0x03) << 5, err);
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

    void At86rf215::set_lna_bypassed(Transceiver transceiver, bool lna_bypass,
                                     Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t aux = spi_read_8(regaux, err) & 0x7F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regaux, aux | (static_cast<uint8_t>(lna_bypass) << 7), err);
    }

    bool At86rf215::get_lna_bypassed(Transceiver transceiver, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t lna_bypass = spi_read_8(regaux, err) & 0x80;
        if (err != Error::NO_ERRORS)
            return 0;
        return lna_bypass >> 7;
    }

    void At86rf215::set_agcmap(Transceiver transceiver,
                               AutomaticGainControlMAP agcmap, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t aux = spi_read_8(regaux, err) & 0x9F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regaux, aux | (static_cast<uint8_t>(agcmap) << 5), err);
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

    void At86rf215::set_external_analog_voltage(Transceiver transceiver,
                                                AutomaticVoltageExternal avext, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t aux = spi_read_8(regaux, err) & 0xEF;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regaux, aux | (static_cast<uint8_t>(avext) << 4), err);
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

    void At86rf215::set_analog_voltage_regulator_enable(Transceiver transceiver,
                                                        bool aven, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t aux = spi_read_8(regaux, err) & 0xF7;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regaux, aux | (static_cast<uint8_t>(aven) << 3), err);
    }

    bool At86rf215::get_analog_voltage_regulator_enable(Transceiver transceiver,
                                                        Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t aven = spi_read_8(regaux, err) & 0x08;
        if (err != Error::NO_ERRORS)
            return 0;
        return aven >> 3;
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

    void At86rf215::set_analog_power_amplifier_voltage(Transceiver transceiver,
                                                       PowerAmplifierVoltageControl pavc, Error& err) {
        RegisterAddress regaux =
            (transceiver == RF09) ? RegisterAddress::RF09_AUXS : RegisterAddress::RF24_AUXS;
        uint8_t aux = spi_read_8(regaux, err) & 0xFC;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regaux, aux | static_cast<uint8_t>(pavc), err);
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

    void At86rf215::set_pa_out_power(Transceiver transceiver, uint8_t gain,
                                     Error& err) {
        RegisterAddress regpac;

        if (transceiver == RF09) {
            regpac = RF09_PAC;
        } else if (transceiver == RF24) {
            regpac = RF24_PAC;
        }

        uint8_t txpa = spi_read_8(regpac, err) & 0xE0;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regpac, txpa | (gain & 0x1F), err);
    }

    void At86rf215::set_mr_oqpsk_rxo(Transceiver transceiver,
                                     RXOOverride receiver_override, Error& err) {
        RegisterAddress regoverride;

        if (transceiver == RF09) {
            regoverride = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            regoverride = BBC1_OQPSKC1;
        }

        uint8_t mr_oqpskc1_reg_masked = spi_read_8(regoverride, err) & 0x7F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regoverride,
                    (static_cast<uint8_t>(receiver_override) << 7) | mr_oqpskc1_reg_masked, err);
    }

    RXOOverride At86rf215::get_mr_oqpsk_rxo(Transceiver transceiver, Error& err) {
        RegisterAddress regoverride;

        if (transceiver == RF09) {
            regoverride = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            regoverride = BBC1_OQPSKC1;
        }

        // if (err != Error::NO_ERRORS) ...
        return static_cast<RXOOverride>(spi_read_8(regoverride, err) >> 7);
    }

    void At86rf215::set_legacy_oqpsk_rxo(Transceiver transceiver,
                                         RXOLEGOverride receiver_override, Error& err) {
        RegisterAddress regoverride;

        if (transceiver == RF09) {
            regoverride = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            regoverride = BBC1_OQPSKC1;
        }

        uint8_t legacy_oqpskc1_reg_masked = spi_read_8(regoverride, err) & 0xBF;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regoverride,
                    (static_cast<uint8_t>(receiver_override) << 6) | legacy_oqpskc1_reg_masked, err);
    }

    RXOLEGOverride At86rf215::get_legacy_oqpsk_rxo(Transceiver transceiver,
                                                   Error& err) {
        RegisterAddress regoverride;

        if (transceiver == RF09) {
            regoverride = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            regoverride = BBC1_OQPSKC1;
        }

        // if (err != Error::NO_ERRORS) ...
        return static_cast<RXOLEGOverride>((spi_read_8(regoverride, err) >> 6) & 0x01); // The mask gets rid of the 7th bit
    }

    void At86rf215::set_preamble_detection_threshold_1(Transceiver transceiver,
                                                       uint8_t threshold, Error& err) {
        RegisterAddress legacy_regthreshold;

        if (threshold != (threshold & 0x07)) {
            // Not the best choice for an error, but there is not a suitable one for this case
            err = Error::UKNOWN_REQUESTED_STATE;
            return;
        }

        if (transceiver == RF09) {
            legacy_regthreshold = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            legacy_regthreshold = BBC1_OQPSKC1;
        }

        uint8_t legacy_oqpskc1_reg_masked = spi_read_8(legacy_regthreshold, err) & 0xC7;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(legacy_regthreshold,
                    (threshold << 3) | legacy_oqpskc1_reg_masked, err);
    }

    uint8_t At86rf215::get_preamble_detection_threshold_1(Transceiver transceiver,
                                                          Error& err) {
        RegisterAddress legacy_regthreshold;

        if (transceiver == RF09) {
            legacy_regthreshold = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            legacy_regthreshold = BBC1_OQPSKC1;
        }

        // if (err != Error::NO_ERRORS) ...
        return (spi_read_8(legacy_regthreshold, err) & 0x38) >> 3;
    }

    void At86rf215::set_preamble_detection_threshold_0(Transceiver transceiver,
                                                       uint8_t threshold, Error& err) {
        RegisterAddress regthreshold;

        if (threshold != (threshold & 0x07)) {
            // Not the best choice for an error, but there is not a suitable one for this case
            err = Error::UKNOWN_REQUESTED_STATE;
            return;
        }

        if (transceiver == RF09) {
            regthreshold = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            regthreshold = BBC1_OQPSKC1;
        }

        uint8_t mr_oqpskc1_reg_masked = spi_read_8(regthreshold, err) & 0xF8;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regthreshold, threshold | mr_oqpskc1_reg_masked, err);
    }

    uint8_t At86rf215::get_preamble_detection_threshold_0(Transceiver transceiver,
                                                          Error& err) {
        RegisterAddress regthreshold;

        if (transceiver == RF09) {
            regthreshold = BBC0_OQPSKC1;
        } else if (transceiver == RF24) {
            regthreshold = BBC1_OQPSKC1;
        }

        // if (err != Error::NO_ERRORS) ...
        return spi_read_8(regthreshold, err) & 0x07;
    }

    uint8_t At86rf215::get_pa_out_power(Transceiver transceiver, Error& err) {
        RegisterAddress regpac;

        if (transceiver == RF09) {
            regpac = RF09_PAC;
        } else if (transceiver == RF24) {
            regpac = RF24_PAC;
        }

        return spi_read_8(regpac, err) & 0x1F;
    }

    void At86rf215::set_oqpsk_rx_spurious_compensation(Transceiver transceiver,
                                                       RXSpuriousCompensation spc, Error& err) {
        RegisterAddress regcomp;
        if (transceiver == RF09) {
            regcomp = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regcomp = BBC1_OQPSKC2;
        }
        uint8_t comp = spi_read_8(regcomp, err) & 0x1F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regcomp, comp | (static_cast<uint8_t>(spc) << 5), err);
    }

    RXSpuriousCompensation At86rf215::get_oqpsk_spurious_compensation(
        Transceiver transceiver, Error& err) {
        RegisterAddress regcomp;
        if (transceiver == RF09) {
            regcomp = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regcomp = BBC1_OQPSKC2;
        }
        return (static_cast<RXSpuriousCompensation>((spi_read_8(regcomp, err) & 0x20) >> 5));
    }

    void At86rf215::set_oqpsk_reduce_power_consumption(Transceiver transceiver,
                                                       ReducePowerConsumption rps, Error& err) {
        RegisterAddress regsave;
        if (transceiver == RF09) {
            regsave = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regsave = BBC1_OQPSKC2;
        }
        uint8_t cons = spi_read_8(regsave, err) & 0x2F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regsave, cons | (static_cast<uint8_t>(rps) << 4), err);
    }

    ReducePowerConsumption At86rf215::get_oqpsk_reduce_power_consumption(
        Transceiver transceiver, Error& err) {
        RegisterAddress regsave;
        if (transceiver == RF09) {
            regsave = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regsave = BBC1_OQPSKC2;
        }
        return (static_cast<ReducePowerConsumption>((spi_read_8(regsave, err) & 0x10) >> 4));
    }

    void At86rf215::set_oqpsk_enable_proprietary_modes(Transceiver transceiver,
                                                       EnableProprietaryModes enprop, Error& err) {
        RegisterAddress regprop;
        if (transceiver == RF09) {
            regprop = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regprop = BBC1_OQPSKC2;
        }
        uint8_t prmodes = spi_read_8(regprop, err) & 0x37;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regprop, prmodes | (static_cast<uint8_t>(enprop) << 3), err);
    }

    void At86rf215::set_high_rate_legacy_oqpsk(Transceiver transceiver,
                                               HighRateLegacyOQPSK hrl, Error& err) {
        RegisterAddress oqpskc3;

        if (transceiver == RF09) {
            oqpskc3 = BBC0_OQPSKC3;
        } else if (transceiver == RF24) {
            oqpskc3 = BBC1_OQPSKC3;
        }

        uint8_t hrloq = spi_read_8(oqpskc3, err) & 0x1F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(oqpskc3, static_cast<uint8_t>(hrl) << 5 | hrloq, err);
    }

    HighRateLegacyOQPSK At86rf215::get_high_rate_legacy_oqpsk(
        Transceiver transceiver, Error& err) {
        RegisterAddress oqpskc3;

        if (transceiver == RF09) {
            oqpskc3 = BBC0_OQPSKC3;
        } else if (transceiver == RF24) {
            oqpskc3 = BBC1_OQPSKC3;
        }

        uint8_t hrlq = spi_read_8(oqpskc3, err) & 0x20;
        return static_cast<HighRateLegacyOQPSK>(hrlq >> 5);
    }

    void At86rf215::set_sfd_search_space(Transceiver transceiver,
                                         SFDSearchSpace sfd, Error& err) {
        RegisterAddress oqpskc3;

        if (transceiver == RF09) {
            oqpskc3 = BBC0_OQPSKC3;
        } else if (transceiver == RF24) {
            oqpskc3 = BBC1_OQPSKC3;
        }

        uint8_t sfdss = spi_read_8(oqpskc3, err) & 0xF3;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(oqpskc3, (static_cast<uint8_t>(sfd) << 2) | sfdss, err);
    }

    SFDSearchSpace At86rf215::get_sfd_search_space(Transceiver transceiver,
                                                   Error& err) {
        RegisterAddress oqpskc3;

        if (transceiver == RF09) {
            oqpskc3 = BBC0_OQPSKC3;
        } else if (transceiver == RF24) {
            oqpskc3 = BBC1_OQPSKC3;
        }

        uint8_t sfdss = spi_read_8(oqpskc3, err) & 0x0C;
        return static_cast<SFDSearchSpace>(sfdss >> 2);
    }

    EnableProprietaryModes At86rf215::get_oqpsk_enable_proprietary_modes(
        Transceiver transceiver, Error& err) {
        RegisterAddress regprop;
        if (transceiver == RF09) {
            regprop = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regprop = BBC1_OQPSKC2;
        }
        return (static_cast<EnableProprietaryModes>((spi_read_8(regprop, err) & 0x8) >> 3));
    }

    void At86rf215::set_oqpsk_fcs_type_for_legacy_oqpsk(Transceiver transceiver,
                                                        FCSType fcstleg, Error& err) {
        RegisterAddress regfcs;
        if (transceiver == RF09) {
            regfcs = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regfcs = BBC1_OQPSKC2;
        }
        uint8_t fcstype = spi_read_8(regfcs, err) & 0x3B;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regfcs, fcstype | (static_cast<uint8_t>(fcstleg) << 2), err);
    }

    FCSType At86rf215::get_oqpsk_fcs_type_for_legacy_oqpsk(Transceiver transceiver,
                                                           Error& err) {
        RegisterAddress regfcs;
        if (transceiver == RF09) {
            regfcs = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regfcs = BBC1_OQPSKC2;
        }
        return (static_cast<FCSType>((spi_read_8(regfcs, err) & 0x4) >> 2));
    }

    void At86rf215::set_oqpsk_receive_mode(Transceiver transceiver, ReceiveMode rxm,
                                           Error& err) {
        RegisterAddress regrec;
        if (transceiver == RF09) {
            regrec = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regrec = BBC1_OQPSKC2;
        }
        uint8_t rxmode = spi_read_8(regrec, err) & 0x3C;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regrec, rxmode | (static_cast<uint8_t>(rxm)), err);
    }

    ReceiveMode At86rf215::get_oqpsk_receive_mode(Transceiver transceiver,
                                                  Error& err) {
        RegisterAddress regrec;
        if (transceiver == RF09) {
            regrec = BBC0_OQPSKC2;
        } else if (transceiver == RF24) {
            regrec = BBC1_OQPSKC2;
        }
        return (static_cast<ReceiveMode>(spi_read_8(regrec, err) & 0x3));
    }

    void At86rf215::set_oqpsk_direct_modulation(Transceiver transceiver,
                                                bool dm_enabled, Error& err) {
        RegisterAddress regoqpskc0;
        if (transceiver == RF09) {
            regoqpskc0 = BBC0_OQPSKC0;
        } else if (transceiver == RF24) {
            regoqpskc0 = BBC1_OQPSKC0;
        }

        uint8_t dirmod = spi_read_8(regoqpskc0, err) & 0xEF;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regoqpskc0, (static_cast<uint8_t>(dm_enabled) << 4) | dirmod,
                    err);
    }

    bool At86rf215::get_oqpsk_direct_modulation(Transceiver transceiver,
                                                Error& err) {
        RegisterAddress regoqpskc0;
        if (transceiver == RF09) {
            regoqpskc0 = BBC0_OQPSKC0;
        } else if (transceiver == RF24) {
            regoqpskc0 = BBC1_OQPSKC0;
        }
        uint8_t dirmod = spi_read_8(regoqpskc0, err) & 0x10;
        if (err != Error::NO_ERRORS)
            return 0;
        return dirmod >> 4;
    }

    void At86rf215::set_oqpsk_modulation(Transceiver transceiver,
                                         OQPSKPulseShapingFilter impulse_response, Error& err) {
        RegisterAddress regoqpskc0;
        if (transceiver == RF09) {
            regoqpskc0 = BBC0_OQPSKC0;
        } else if (transceiver == RF24) {
            regoqpskc0 = BBC1_OQPSKC0;
        }

        uint8_t mod = spi_read_8(regoqpskc0, err) & 0xF7;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regoqpskc0, (static_cast<uint8_t>(impulse_response) << 3) | mod,
                    err);
    }

    OQPSKPulseShapingFilter At86rf215::get_oqpsk_modulation(Transceiver transceiver,
                                                            Error& err) {
        RegisterAddress regoqpskc0;
        if (transceiver == RF09) {
            regoqpskc0 = BBC0_OQPSKC0;
        } else if (transceiver == RF24) {
            regoqpskc0 = BBC1_OQPSKC0;
        }
        uint8_t mod = spi_read_8(regoqpskc0, err) & 0x08;
        if (err != Error::NO_ERRORS)
            return static_cast<OQPSKPulseShapingFilter>(0);
        return static_cast<OQPSKPulseShapingFilter>(mod >> 3);
    }

    void At86rf215::set_oqpsk_chip_frequency(Transceiver transceiver,
                                             OQPSKChipFrequency chip_frequency, Error& err) {
        RegisterAddress regoqpskc0;
        if (transceiver == RF09) {
            regoqpskc0 = BBC0_OQPSKC0;
        } else if (transceiver == RF24) {
            regoqpskc0 = BBC1_OQPSKC0;
        }

        uint8_t freq = spi_read_8(regoqpskc0, err) & 0xFC;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(regoqpskc0, (static_cast<uint8_t>(chip_frequency)) | freq, err);
    }

    OQPSKChipFrequency At86rf215::get_oqpsk_chip_frequency(Transceiver transceiver,
                                                           Error& err) {
        RegisterAddress regoqpskc0;
        if (transceiver == RF09) {
            regoqpskc0 = BBC0_OQPSKC0;
        } else if (transceiver == RF24) {
            regoqpskc0 = BBC1_OQPSKC0;
        }
        uint8_t freq = spi_read_8(regoqpskc0, err) & 0x03;
        if (err != Error::NO_ERRORS)
            return OQPSKChipFrequency::INVALID;
        return static_cast<OQPSKChipFrequency>(freq);
    }

    int8_t At86rf215::get_rssi(Transceiver transceiver, Error& err) {
        RegisterAddress regrssi;

        if (transceiver == RF09) {
            regrssi = RF09_RSSI;
        } else if (transceiver == RF24) {
            regrssi = RF24_RSSI;
        }

        return spi_read_8(regrssi, err);
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

    int8_t At86rf215::get_receiver_energy_detection(Transceiver transceiver,
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
            err = Error::ONGOING_TRANSMISSION_RECEPTION;
            return;
        }

        set_state(transceiver, State::RF_TRXOFF, err);
        if (err != Error::NO_ERRORS) {
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

        // write length to register
        spi_write_8(regtxfll, length & 0xFF, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        spi_write_8(regtxflh, (length >> 8) & 0x07, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // write to tx frame buffer

        spi_block_write_8(regfbtxs, length, packet, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        tx_ongoing = true;
        set_state(transceiver, State::RF_TXPREP, err);
    }

    void At86rf215::clear_channel_assessment(Transceiver transceiver, Error& err) {
        if (tx_ongoing or rx_ongoing) {
            err = ONGOING_TRANSMISSION_RECEPTION;
        }
        set_state(transceiver, State::RF_TRXOFF, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        rx_ongoing = true;
        cca_ongoing = true;
        set_state(transceiver, State::RF_TXPREP, err);
    }


    void At86rf215::transmitBasebandPacketsRx(Transceiver transceiver, Error& err) {
        set_state(transceiver, State::RF_TRXOFF, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        set_state(transceiver, State::RF_RX, err);
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


    void At86rf215::set_battery_monitor_status(bool status, Error& err) {
        uint8_t bmdvc = spi_read_8(RF_BMDVC, err) & 0x1F;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(RF_BMDVC, (static_cast<uint8_t>(status) << 5) | bmdvc, err);
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
        BatteryMonitorVoltage threshold, Error& err) {
        uint8_t bmvt = spi_read_8(RF_BMDVC, err) & 0x30;
        if (err != Error::NO_ERRORS)
            return;
        spi_write_8(RF_BMDVC, bmvt | static_cast<uint8_t>(threshold), err);
    }

    uint8_t At86rf215::get_battery_monitor_voltage_threshold(Error& err) {
        return spi_read_8(RF_BMDVC, err) & 0x0F;
    }

    void At86rf215::setup_tx_frontend(Transceiver transceiver,
                                      PowerAmplifierRampTime pa_ramp_time, TransmitterCutOffFrequency cutoff,
                                      TxRelativeCutoffFrequency tx_rel_cutoff, bool direct_mod,
                                      TransmitterSampleRate tx_sample_rate,
                                      PowerAmplifierCurrentControl pa_curr_control, uint8_t tx_out_power,
                                      ExternalLNABypass ext_lna_bypass, AutomaticGainControlMAP agc_map,
                                      AutomaticVoltageExternal avg_ext, AnalogVoltageEnable av_enable,
                                      PowerAmplifierVoltageControl pa_vcontrol, Error& err) {
        RegisterAddress regtxcut;
        RegisterAddress regtxdfe;
        RegisterAddress regpac;
        RegisterAddress regauxs;

        uint8_t reg;

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

        // Set RFn_TXCUTC
        reg = (static_cast<uint8_t>(pa_ramp_time) << 6) | static_cast<uint8_t>(cutoff);
        spi_write_8(regtxcut, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RFn_TXDFE
        reg = (static_cast<uint8_t>(tx_rel_cutoff) << 5) | static_cast<uint8_t>(direct_mod) << 4 | static_cast<uint8_t>(tx_sample_rate);
        spi_write_8(regtxdfe, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RFn_PAC
        reg = (static_cast<uint8_t>(pa_curr_control) << 5) | (tx_out_power & 0x1F);
        spi_write_8(regpac, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RFn_AUXS
        reg = (static_cast<uint8_t>(ext_lna_bypass) << 7) | (static_cast<uint8_t>(agc_map) << 5) | (static_cast<uint8_t>(avg_ext) << 4) | (static_cast<uint8_t>(av_enable) << 3) | (static_cast<uint8_t>(pa_vcontrol));
        spi_write_8(regauxs, reg, err);
    }

    void At86rf215::setup_iq(ExternalLoopback external_loop,
                             IQOutputCurrent out_cur, IQmodeVoltage common_mode_vol,
                             IQmodeVoltageIEE common_mode_iee, EmbeddedControlTX embedded_tx_start,
                             ChipMode chip_mode, SkewAlignment skew_alignment, Error& err) {
        // Set RF_IQIFC0
        uint8_t reg;
        reg = (static_cast<uint8_t>(external_loop) << 7) | (static_cast<uint8_t>(out_cur) << 4) | (static_cast<uint8_t>(common_mode_vol) << 2) | (static_cast<uint8_t>(common_mode_iee) << 1) | static_cast<uint8_t>(embedded_tx_start);
        spi_write_8(RF_IQIFC0, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RF_IQIFC1
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

    void At86rf215::setup_battery(BatteryMonitorVoltage battery_monitor_voltage,
                                  BatteryMonitorHighRange battery_monitor_high_range, Error& err) {
        uint8_t reg = (static_cast<uint8_t>(battery_monitor_high_range) << 4) | static_cast<uint8_t>(battery_monitor_voltage);
        spi_write_8(RF_BMDVC, reg, err);
    }

    void At86rf215::setup_rx_energy_detection(Transceiver transceiver,
                                              EnergyDetectionMode energy_mode, uint8_t energy_detect_factor,
                                              EnergyDetectionTimeBasis energy_time_basis, Error& err) {
        uint8_t reg;
        RegisterAddress regedc;
        RegisterAddress regedd;

        if (transceiver == Transceiver::RF09) {
            regedc = RF09_EDC;
            regedd = RF09_EDD;
        } else if (transceiver == Transceiver::RF24) {
            regedc = RF24_EDC;
            regedd = RF09_EDD;
        }

        // Set RFn_EDC
        spi_write_8(regedc, static_cast<uint8_t>(energy_mode), err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RFn_EDD
        reg = (energy_detect_factor << 2) | static_cast<uint8_t>(energy_time_basis);
        spi_write_8(regedd, reg, err);
    }

    void At86rf215::setup_rx_frontend(Transceiver transceiver, bool if_inversion,
                                      bool if_shift, ReceiverBandwidth rx_bw,
                                      RxRelativeCutoffFrequency rx_rel_cutoff,
                                      ReceiverSampleRate rx_sample_rate, bool agc_input,
                                      AverageTimeNumberSamples agc_avg_sample, bool agc_enabled,
                                      AutomaticGainTarget agc_target, uint8_t gain_control_word, Error& err) {
        if (gain_control_word > 0x23) {
            err = Error::INVALID_AGC_CONTROl_WORD;
            return;
        }

        RegisterAddress regrxbwc;
        RegisterAddress regrxdfe;
        RegisterAddress regagcc;
        RegisterAddress regagcs;

        uint8_t reg;

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

        // Set RFn_RXBWC
        reg = (static_cast<uint8_t>(if_inversion) << 5) | (static_cast<uint8_t>(if_shift) << 4) | static_cast<uint8_t>(rx_bw);
        spi_write_8(regrxbwc, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RFn_RXDFE
        reg = (static_cast<uint8_t>(rx_rel_cutoff) << 5) | static_cast<uint8_t>(rx_sample_rate);
        spi_write_8(regrxdfe, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RFn_AGC
        reg = (static_cast<uint8_t>(agc_input) << 6) | (static_cast<uint8_t>(agc_avg_sample) << 4);
        spi_write_8(regagcc, reg, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set RFn_AGCG
        reg = (static_cast<uint8_t>(agc_target) << 5) | gain_control_word;
        spi_write_8(regagcs, reg, err);
    }

    void At86rf215::setup_irq_cfg(bool maskMode, IRQPolarity polarity,
                                  PadDriverStrength padDriverStrength, Error& err) {
        RegisterAddress regcfg;
        regcfg = RF_CFG;

        spi_write_8(RF_CFG,
                    (maskMode << 3) | (static_cast<uint8_t>(polarity) << 2) | static_cast<uint8_t>(padDriverStrength), err);
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
                                   bool frameBufferLevelIndication, bool agcEnabled, bool agcRelease, bool agcHold,
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
                    frameBufferLevelIndication << 7 | agcEnabled << 6 | agcHold << 5 | transmitterFrameEnd << 4 | receiverExtendedMatch << 3 | receiverAddressMatch << 2 | receiverFrameEnd << 1 | receiverFrameStart, err);
    }

    void At86rf215::setup(Error& err) {
        // Check state of RF09 core
        State state = get_state(Transceiver::RF09, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        // We have access to all registers only if we are in the state TRXOFF
        if (state != State::RF_TRXOFF) {
            err = Error::INVALID_STATE_FOR_OPERATION;
            return;
        }
        // Check state of RF24 core - we only proceed with the set-up if both cores are in the TRXOFF state to avoid setting half the registers
        state = get_state(Transceiver::RF24, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        if (state != State::RF_TRXOFF) {
            err = Error::INVALID_STATE_FOR_OPERATION;
            return;
        }

        // Set IRQ masks
        setup_irq_mask(Transceiver::RF09, radioInterruptsConfig.iqIfSynchronizationFailure09, radioInterruptsConfig.transceiverError09,
                       radioInterruptsConfig.batteryLow09, radioInterruptsConfig.energyDetectionCompletion09, radioInterruptsConfig.transceiverReady09,
                       radioInterruptsConfig.wakeup09, interruptsConfig.frameBufferLevelIndication09, rxConfig.agcEnabled09, interruptsConfig.agcRelease09,
                       interruptsConfig.agcHold09, interruptsConfig.transmitterFrameEnd09, interruptsConfig.receiverExtendedMatch09,
                       interruptsConfig.receiverAddressMatch09, interruptsConfig.receiverFrameEnd09, interruptsConfig.receiverFrameStart09, err);

        setup_irq_mask(Transceiver::RF24, radioInterruptsConfig.iqIfSynchronizationFailure24, radioInterruptsConfig.transceiverError24,
                       radioInterruptsConfig.batteryLow24, radioInterruptsConfig.energyDetectionCompletion24, radioInterruptsConfig.transceiverReady24,
                       radioInterruptsConfig.wakeup24, interruptsConfig.frameBufferLevelIndication24, rxConfig.agcEnabled24, interruptsConfig.agcRelease24,
                       interruptsConfig.agcHold24, interruptsConfig.transmitterFrameEnd24, interruptsConfig.receiverExtendedMatch24,
                       interruptsConfig.receiverAddressMatch24, interruptsConfig.receiverFrameEnd24, interruptsConfig.receiverFrameStart24, err);

        // Set IRQ pin
        setup_irq_cfg(config.irqMaskMode, config.irqPolarity,
                      config.padDriverStrength, err);

        // Set PLL
        configure_pll(Transceiver::RF09, freqSynthesizerConfig.channelCenterFrequency09,
                      freqSynthesizerConfig.channelNumber09, freqSynthesizerConfig.channelMode09,
                      freqSynthesizerConfig.loopBandwidth09, freqSynthesizerConfig.channelSpacing09, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        // TO do configure PLL for RF24

        // Setup Physical Layer for Baseband Cores
        setup_phy_baseband(Transceiver::RF09, basebandCoreConfig.continuousTransmit09,
                           basebandCoreConfig.frameCheckSequenceFilterEn09, basebandCoreConfig.transmitterAutoFrameCheckSequence09,
                           basebandCoreConfig.frameCheckSequenceType09, basebandCoreConfig.baseBandEnable09,
                           basebandCoreConfig.physicalLayerType09, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        setup_phy_baseband(Transceiver::RF24, basebandCoreConfig.continuousTransmit24,
                           basebandCoreConfig.frameCheckSequenceFilterEn24, basebandCoreConfig.transmitterAutoFrameCheckSequence24,
                           basebandCoreConfig.frameCheckSequenceType24, basebandCoreConfig.baseBandEnable24,
                           basebandCoreConfig.physicalLayerType24, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set TX front-end
        setup_tx_frontend(Transceiver::RF09, txConfig.powerAmplifierRampTime09,
                          txConfig.transmitterCutOffFrequency09,
                          txConfig.txRelativeCutoffFrequency09, txConfig.directModulation09,
                          txConfig.transceiverSampleRate09,
                          txConfig.powerAmplifierCurrentControl09, txConfig.txOutPower09,
                          auxilarySettings.externalLNABypass09, auxilarySettings.automaticGainControlMAP09,
                          auxilarySettings.automaticVoltageExternal09, auxilarySettings.analogVoltageEnable09,
                          auxilarySettings.powerAmplifierVoltageControl09, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        setup_tx_frontend(Transceiver::RF24, txConfig.powerAmplifierRampTime24,
                          txConfig.transmitterCutOffFrequency24,
                          txConfig.txRelativeCutoffFrequency24, txConfig.directModulation24,
                          txConfig.transceiverSampleRate24,
                          txConfig.powerAmplifierCurrentControl24, txConfig.txOutPower24,
                          auxilarySettings.externalLNABypass24, auxilarySettings.automaticGainControlMAP24,
                          auxilarySettings.automaticVoltageExternal24, auxilarySettings.analogVoltageEnable24,
                          auxilarySettings.powerAmplifierVoltageControl24, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set up RX front-end
        setup_rx_frontend(Transceiver::RF09, rxConfig.ifInversion09, rxConfig.ifShift09,
                          rxConfig.receiverBandwidth09, rxConfig.rxRelativeCutoffFrequency09,
                          rxConfig.receiverSampleRate09, rxConfig.agcInput09,
                          rxConfig.averageTimeNumberSamples09, rxConfig.agcEnabled09,
                          rxConfig.automaticGainTarget09, rxConfig.gainControlWord09, err);
        if (err != Error::NO_ERRORS) {
            return;
        }
        setup_rx_frontend(Transceiver::RF24, rxConfig.ifInversion24, rxConfig.ifShift24,
                          rxConfig.receiverBandwidth24, rxConfig.rxRelativeCutoffFrequency24,
                          rxConfig.receiverSampleRate24, rxConfig.agcInput24,
                          rxConfig.averageTimeNumberSamples24, rxConfig.agcEnabled24,
                          rxConfig.automaticGainTarget24, rxConfig.gainControlWord24, err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set up IQ interface
        setup_iq(iqInterfaceConfig.externalLoopback, iqInterfaceConfig.iqOutputCurrent,
                 iqInterfaceConfig.iqmodeVoltage, iqInterfaceConfig.iqmodeVoltageIEE,
                 iqInterfaceConfig.embeddedControlTX, iqInterfaceConfig.chipMode, iqInterfaceConfig.skewAlignment,
                 err);
        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set up energy detection // RFn_EDC, RFn_EDD
        setup_rx_energy_detection(Transceiver::RF09, rxConfig.energyDetectionMode09,
                                  rxConfig.energyDetectFactor09, rxConfig.energyDetectionBasis09, err);
        setup_rx_energy_detection(Transceiver::RF24, rxConfig.energyDetectionMode24,
                                  rxConfig.energyDetectFactor24, rxConfig.energyDetectionBasis24, err);

        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set up battery
        setup_battery(config.batteryMonitorVoltage, config.batteryMonitorHighRange,
                      err);

        if (err != Error::NO_ERRORS) {
            return;
        }

        // Set up crystal oscillator
        setup_crystal(config.fastStartUp, config.crystalTrim, err);
    }

    uint8_t At86rf215::get_irq(Transceiver transceiver, Error& err) {
        if (transceiver == RF09) {
            return spi_read_8(RF09_IRQS, err);
        } else if (transceiver == RF24) {
            return spi_read_8(RF24_IRQS, err);
        }
        return 0;
    }

    void At86rf215::handle_irq(void) {
        Error err = Error::NO_ERRORS;

        /* Sub 1-GHz Transceiver */

        // Radio IRQ
        volatile uint8_t irq = spi_read_8(RegisterAddress::RF09_IRQS, err);
        if ((irq & InterruptMask::IFSynchronization) != 0) {
            // I/Q IF Synchronization Failure handling
            IFSynchronization_flag = true;
            xTaskNotify(rf_txtask->taskHandle, 0, eSetBits);
        }
        if ((irq & InterruptMask::TransceiverError) != 0) {
            // Transceiver Error handling
            TransceiverError_flag = true;
        }
        if ((irq & InterruptMask::BatteryLow) != 0) {
            // Battery Low handling
        }
        if ((irq & InterruptMask::EnergyDetectionCompletion) != 0) {
            EnergyDetectionCompletion_flag = true;
            rx_ongoing = false;
            cca_ongoing = false;
            energy_measurement = get_receiver_energy_detection(Transceiver::RF09, err);
        }
        if ((irq & InterruptMask::TransceiverReady) != 0) {
            TransceiverReady_flag = true;
            if (rx_ongoing) {
                // Switch to TX state once the transceiver is ready to send
                set_state(Transceiver::RF09, State::RF_RX, err);
                if (cca_ongoing) {
                    spi_write_8(RF09_EDC, 0x1, err);
                }
            }
            if (tx_ongoing) {
                // Switch to TX state once the transceiver is ready to send
                set_state(Transceiver::RF09, State::RF_TX, err);
            }
        }
        if ((irq & InterruptMask::Wakeup) != 0) {
            Wakeup_flag = true;
            // Wakeup handling
        }

        //Baseband IRQ
        irq = spi_read_8(RegisterAddress::BBC0_IRQS, err);
        if ((irq & InterruptMask::FrameBufferLevelIndication) != 0) {
            // Frame Buffer Level Indication handling
            FrameBufferLevelIndication_flag = true;
        }
        if ((irq & InterruptMask::AGCRelease) != 0) {
            // AGC Release handling
            AGCRelease_flag = true;
        }
        if ((irq & InterruptMask::AGCHold) != 0) {
            // AGC Hold handling
        }
        if ((irq & InterruptMask::TransmitterFrameEnd) != 0) {
            TransmitterFrameEnd_flag = true;
            tx_ongoing = false;
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
            if (rx_ongoing) {
                //                packetReception(Transceiver::RF09, err); // hard_fault if enabled for packets above 106 bytes
                rx_ongoing = false;
            }
        }
        if ((irq & InterruptMask::ReceiverFrameStart) != 0) {
            ReceiverFrameStart_flag = true;
            rx_ongoing = true;
        }

        /* 2.4 GHz Transceiver */

        // Radio IRQ
        irq = spi_read_8(RegisterAddress::RF24_IRQS, err);

        if ((irq & InterruptMask::IFSynchronization) != 0) {
            // I/Q IF Synchronization Failure handling
        }
        if ((irq & InterruptMask::TransceiverError) != 0) {
            // Transceiver Error handling
        }
        if ((irq & InterruptMask::BatteryLow) != 0) {
            // Battery Low handling
        }
        if ((irq & InterruptMask::EnergyDetectionCompletion) != 0) {
            rx_ongoing = false;
            cca_ongoing = false;
            energy_measurement = get_receiver_energy_detection(Transceiver::RF24, err);
        }
        if ((irq & InterruptMask::TransceiverReady) != 0) {
            if (rx_ongoing) {
                // Switch to TX state once the transceiver is ready to send
                set_state(Transceiver::RF24, State::RF_RX, err);
                if (cca_ongoing) {
                    spi_write_8(RF24_EDC, 0x1, err);
                }
            }
            if (tx_ongoing) {
                // Switch to TX state once the transceiver is ready to send
                set_state(Transceiver::RF24, State::RF_TX, err);
            }
        }
        if ((irq & InterruptMask::Wakeup) != 0) {
            // Wakeup handling
        }

        //Baseband IRQ
        irq = spi_read_8(RegisterAddress::BBC0_IRQS, err);
        if ((irq & InterruptMask::FrameBufferLevelIndication) != 0) {
            // Frame Buffer Level Indication handling
        }
        if ((irq & InterruptMask::AGCRelease) != 0) {
            // AGC Release handling
        }
        if ((irq & InterruptMask::AGCHold) != 0) {
            agc_held = true;
        }
        if ((irq & InterruptMask::TransmitterFrameEnd) != 0) {
            tx_ongoing = false;
        }
        if ((irq & InterruptMask::ReceiverExtendMatch) != 0) {
            // Receiver Extended Match handling
        }
        if ((irq & InterruptMask::ReceiverAddressMatch) != 0) {
            // Receiver Address Match handling
        }
        if ((irq & InterruptMask::ReceiverFrameEnd) != 0) {
            if (rx_ongoing) {
                packetReception(Transceiver::RF24, err);
                rx_ongoing = false;
            }
        }
        if ((irq & InterruptMask::ReceiverFrameStart) != 0) {
            // Receiver Frame Start handling
            rx_ongoing = true;
        }
    }
} // namespace AT86RF215