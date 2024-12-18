
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


etl::expected<void, Error> RF_RXTask::check_transceiver_connection() {
    DevicePartNumber dpn = transceiver.get_part_number(error);
    if (error == AT86RF215::Error::NO_ERRORS && dpn == DevicePartNumber::AT86RF215) {
        LOG_DEBUG << "RF_PN: " << transceiver.spi_read_8(RF_PN, error);
        return {}; /// success
    }

    else
        return etl::unexpected<Error>(error);
}


void RF_RXTask::execute() {
    LOG_INFO << "RF RX TASK";
    /// Set the Up-link frequency
    transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
    transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
    /// Try to enable the 5V right away from the CubeMX
    vTaskDelay(pdMS_TO_TICKS(5000));
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
    //    LOG_INFO << "RF 5V ENABLED";
    /// Check transceiver connection
    //    auto status = check_transceiver_connection();
    //    if(status.has_value()){
    //        LOG_INFO << "TRANSCEIVER CONNECTION OK";
    //    }
    //    else
    //        LOG_ERROR << "TRANSCEIVER ERROR" << status.error();
    //    transceiver.setup(error);
    while (1) {
    }
}
