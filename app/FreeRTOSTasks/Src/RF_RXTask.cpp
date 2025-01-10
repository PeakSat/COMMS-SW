#include "RF_RXTask.hpp"
#include "RF_TXTask.hpp"
#include "Logger.hpp"

using namespace AT86RF215;

void RF_RXTask::execute() {
    vTaskDelay(5000);
    LOG_INFO << "RF RX TASK";
    /// Set the Up-link frequency
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    LOG_INFO << "5V CHANNEL ON";
    // ENABLE THE RX SWITCH
    HAL_GPIO_WritePin(EN_RX_UHF_GPIO_Port, EN_RX_UHF_Pin, GPIO_PIN_RESET);
    LOG_DEBUG << "RX SWITCH ENABLED ";
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_RESET);
    LOG_DEBUG << "RX AMP ENABLED ";
    /// Essential for the trx to be able to send and receive packets
    /// (If you have it HIGH from the CubeMX the trx will not be able to send)
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
    vTaskDelay(20);
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
    transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
    /// Check transceiver connection
    if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
        auto status = transceiver.check_transceiver_connection(error);
        if (status.has_value()) {
            LOG_INFO << "AT86RF215 CONNECTION OK";
        } else
            /// TODO: Error handling
            LOG_ERROR << "AT86RF215 ##ERROR## WITH CODE: " << status.error();
        transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
        transceiver.setup(error);
        xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
    }
    uint16_t received_length;
    transceiver.set_state(AT86RF215::RF09, State::RF_TXPREP, error);
    vTaskDelay(pdMS_TO_TICKS(20));
    transceiver.set_state(AT86RF215::RF09, State::RF_RX, error);
    if (transceiver.get_state(RF09, error) == RF_RX)
        LOG_INFO << "STATE = RX";
    uint32_t receivedEvents = 0;
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_SET);
    LOG_DEBUG << "RX AMP ENABLED ";
    while (1) {
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &receivedEvents, 5000)) {
            if (receivedEvents & RXFE) {
                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    if (result.has_value()) {
                        received_length = result.value();
                        LOG_INFO << "RX PACKET WITH RECEPTION LENGTH: " << received_length;
                        LOG_INFO << "Counter packet: " << transceiver.spi_read_8((AT86RF215::BBC0_FBRXS), error);
                    } else {
                        Error err = result.error();
                        LOG_ERROR << "AT86RF215 ##ERROR## AT RX LENGTH RECEPTION WITH CODE: " << err;
                    }
                    vTaskDelay(20);
                    transceiver.set_state(AT86RF215::RF09, State::RF_RX, error);
                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                }
            }
            if (receivedEvents & AGC_HOLD && (receivedEvents)) {
                LOG_INFO << "RSSI [AGC HOLD]: " << transceiver.get_rssi(RF09, error);
            }
        }
    }
}