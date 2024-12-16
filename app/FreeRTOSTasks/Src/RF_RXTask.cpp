#include "RF_RXTask.hpp"
#include "Logger.hpp"

using namespace AT86RF215;

void RF_RXTask::setRFmode(uint8_t mode) {
    if (mode) {
        transceiver.set_state(AT86RF215::RF09, State::RF_TXPREP, error);
        vTaskDelay(pdMS_TO_TICKS(10));
        transceiver.set_state(AT86RF215::RF09, State::RF_RX, error);
        if (transceiver.get_state(AT86RF215::RF09, error) == (AT86RF215::State::RF_RX))
            LOG_DEBUG << " STATE = RX ";
    } else {
        transceiver.set_state(AT86RF215::RF09, State::RF_TX, error);
        transceiver.TransmitterFrameEnd_flag = true;
    }
}

uint8_t RF_RXTask::checkTheSPI() {
    uint8_t spi_error = 0;
    DevicePartNumber dpn = transceiver.get_part_number(error);
    etl::string<LOGGER_MAX_MESSAGE_SIZE> output;
    if (dpn == DevicePartNumber::AT86RF215) {
        output = "SPI IS OK";
        Logger::log(Logger::debug, output);
    }

    else {
        spi_error = 1;
        LOG_DEBUG << "SPI ERROR";
        transceiver.chip_reset(error);
    }
    return spi_error;
}


void RF_RXTask::execute() {
    LOG_DEBUG << "RF RX TASK";
    // set up-link frequency
    transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
    transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
    /// try to enable the 5V right away from the cubemx
    vTaskDelay(pdMS_TO_TICKS(5000));
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    LOG_DEBUG << "RF 5V ENABLED";
    /// Check the SPI connection
    while (checkTheSPI() != 0) {
        vTaskDelay(10);
    };
    uint8_t read_reg;
    transceiver.setup(error);
    ///
    setRFmode(RX);
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_PC, error);
    LOG_DEBUG << "BBC0_PC = " << read_reg;
    // RX DIGITAL FRONT END  //
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKC0, error);
    LOG_DEBUG << "BBC0_FSKC0" << read_reg;
    //
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKC1, error);
    LOG_DEBUG << "BBC0_FSKC1 = " << read_reg;
    // RX AGC and Receiver Gain
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKC2, error);
    LOG_DEBUG << "BBC0_FSKC2 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKC3, error);
    LOG_DEBUG << "BBC0_FSKC3 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKC4, error);
    LOG_DEBUG << "BBC0_FSKC4 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKPLL, error);
    LOG_DEBUG << "BBC0_FSKPLL = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKSFD0L, error);
    LOG_DEBUG << "BBC0_FSKSFD0L = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKSFD0H, error);
    LOG_DEBUG << "BBC0_FSKSFD0H = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKSFD1L, error);
    LOG_DEBUG << "BBC0_FSKSFD1L = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKSFD1H, error);
    LOG_DEBUG << "BBC0_FSKSFD1H = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKPHRTX, error);
    LOG_DEBUG << "BBC0_FSKPHRTX = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKPHRRX, error);
    LOG_DEBUG << "BBC0_FSKPHRRX = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKRRXFLL, error);
    LOG_DEBUG << "BBC0_FSKRRXFLL = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKRRXFLH, error);
    LOG_DEBUG << "BBC0_FSKRRXFLH = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKRPC, error);
    LOG_DEBUG << "BBC0_FSKRPC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKRPCONT, error);
    LOG_DEBUG << "BBC0_FSKRPCONT = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKRPCOFFT, error);
    LOG_DEBUG << "BBC0_FSKRPCOFFT = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKDM, error);
    LOG_DEBUG << "BBC0_FSKDM = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKPE0, error);
    LOG_DEBUG << "BBC0_FSKPE0 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKPE1, error);
    LOG_DEBUG << "BBC0_FSKPE1 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKPE2, error);
    LOG_DEBUG << "BBC0_FSKPE2 = " << read_reg;
    LOG_DEBUG << "TRANSMITTER DIGITAL FRONT END";
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXDFE, error);
    LOG_DEBUG << "RF09_TXDFE = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXCUTC, error);
    LOG_DEBUG << "RF09_TXCUTC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PAC, error);
    LOG_DEBUG << "RF09_PAC = " << read_reg;
    //
    LOG_DEBUG << "AUXILARY SETTING";
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_AUXS, error);
    LOG_DEBUG << "RF09_AUXS = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PADFE, error);
    LOG_DEBUG << "RF09_PADFE = " << read_reg;
    // receiver
    LOG_DEBUG << "RECEIVER";
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_RXBWC, error);
    LOG_DEBUG << "RF09_RXBWC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_RXDFE, error);
    LOG_DEBUG << "RF09_RXDFE = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_AGCC, error);
    LOG_DEBUG << "RF09_AGCC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_AGCS, error);
    LOG_DEBUG << "RF09_AGCS = " << read_reg;
    int read_rssi = transceiver.int_spi_read_8(AT86RF215::RF09_RSSI, error);
    LOG_DEBUG << "RF09_RSSI = " << read_rssi;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_EDC, error);
    LOG_DEBUG << "RF09_EDC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_EDD, error);
    LOG_DEBUG << "RF09_EDD = " << read_reg;
    int read_edv = transceiver.int_spi_read_8(AT86RF215::RF09_EDV, error);
    LOG_DEBUG << "RF09_EDV = " << read_edv;
    // PLL REGS
    LOG_DEBUG << "PLL REGS";
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_CS, error);
    LOG_DEBUG << "RF09_CS = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_CCF0L, error);
    LOG_DEBUG << "RF09_CCF0L = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_CCF0H, error);
    LOG_DEBUG << "RF09_CCF0H = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_CNL, error);
    LOG_DEBUG << "RF09_CNL = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_CNM, error);
    LOG_DEBUG << "RF09_CNM = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PLL, error);
    LOG_DEBUG << "RF09_PLL = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PLLCF, error);
    LOG_DEBUG << "RF09_PLLCF = " << read_reg;
    // IRQ
    LOG_DEBUG << "IRQ ";
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_IRQM, error);
    LOG_DEBUG << "BBC0_IRQM = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_IRQS, error);
    LOG_DEBUG << "BBC0_IRQS = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_IRQM, error);
    LOG_DEBUG << "RF09_IRQM = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_IRQS, error);
    LOG_DEBUG << "RF09_IRQS = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF_CFG, error);
    LOG_DEBUG << "RF_CFG = " << read_reg;
    //
    LOG_DEBUG << "Phase Measurement Unit";
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_PMUC, error);
    LOG_DEBUG << "BBC0_PMUC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_PMUVAL, error);
    LOG_DEBUG << "BBC0_PMUVAL = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_PMUQF, error);
    LOG_DEBUG << "BBC0_PMUQF = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_PMUI, error);
    LOG_DEBUG << "BBC0_PMUI = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_PMUQ, error);
    LOG_DEBUG << "BBC0_PMUQ = " << read_reg;
    //
    LOG_DEBUG << "Timestamp Counter";
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_CNTC, error);
    LOG_DEBUG << "BBC0_CNTC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_CNT0, error);
    LOG_DEBUG << "BBC0_CNT0 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_CNT1, error);
    LOG_DEBUG << "BBC0_CNT1 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_CNT2, error);
    LOG_DEBUG << "BBC0_CNT2 = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::BBC0_CNT3, error);
    LOG_DEBUG << "BBC0_CNT3 = " << read_reg;
    //
    LOG_DEBUG << "STATE MACHINE REGS";
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_STATE, error);
    LOG_DEBUG << "Transceiver State = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_CMD, error);
    LOG_DEBUG << "Transceiver command = " << read_reg;
}
