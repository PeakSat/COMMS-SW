#include "RF_TXTask.hpp"
#include "Logger.hpp"


void RF_TXTask::execute() {
    vTaskDelay(10000);
    // set down-link frequency
    /*
    transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
    transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
    uint8_t read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXDFE, error);
    LOG_DEBUG << "RF09_TXDFE = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXCUTC, error);
    LOG_DEBUG << "RF09_TXCUTC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PAC, error);
    LOG_DEBUG << "RF09_PAC = " << read_reg;
    transceiver.setup(error);
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXDFE, error);
    LOG_DEBUG << "RF09_TXDFE = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXCUTC, error);
    LOG_DEBUG << "RF09_TXCUTC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PAC, error);
    LOG_DEBUG << "RF09_PAC = " << read_reg;
    */
}
