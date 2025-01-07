#include "RF_RXTask.hpp"
#include "Logger.hpp"

using namespace AT86RF215;

void RF_RXTask::execute() {
    LOG_INFO << "RF RX TASK";
    /// Set the Up-link frequency
    transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
    /// Don't wait for a long time to power on the 5V channel, because the AT have a leakage power supply of 1.5V due to the 3.3V channel leakage.
    vTaskDelay(pdMS_TO_TICKS(1000));
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    LOG_INFO << "5V CHANNEL ON";
    /// Check transceiver connection
    auto status = transceiver.check_transceiver_connection(error);
    if (status.has_value()) {
        LOG_INFO << "AT86RF215 CONNECTION OK";
    } else
        /// TODO: Error handling
        LOG_ERROR << "AT86RF215 ##ERROR## WITH CODE: " << status.error();
    transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
    transceiver.setup(error);
    uint16_t received_length;
    while (1) {
        uint32_t receivedEvents = 0;
        if (xTaskNotifyWait(RXFE, RXFE, &receivedEvents, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (receivedEvents & RXFE) {
                /// TODO: HANDLE THE RECEIVED PACKET
                LOG_DEBUG << "RECEIVER FRAME END NOTIFICATION";
                auto result = transceiver.get_received_length(RF09, error);
                if (result.has_value()) {
                    received_length = result.value();
                    LOG_INFO << "RX PACKET WITH RECEPTION LENGTH: " << received_length;
                } else {
                    Error err = result.error();
                    LOG_ERROR << "AT86RF215 ##ERROR## AT RX LENGTH RECEPTION WITH CODE: " << err;
                }
            }
        }
        if (xTaskNotifyWait(AGC_HOLD, AGC_HOLD, &receivedEvents, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (receivedEvents & AGC_HOLD) {
                /// TODO: HANDLE THE RECEIVED PACKET
                LOG_DEBUG << "AGC HOLD NOTIFICATION";
                auto result = transceiver.get_received_length(RF09, error);
                if (result.has_value()) {
                    received_length = result.value();
                    LOG_INFO << "RX PACKET WITH RECEPTION LENGTH: " << received_length;
                } else {
                    /// TODO: Error handling
                    Error err = result.error();
                    LOG_ERROR << "AT86RF215 ##ERROR## AT RX LENGTH RECEPTION WITH CODE: " << err;
                }
            }
        }
    }
}
