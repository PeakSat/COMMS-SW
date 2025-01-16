#include "RF_RXTask.hpp"
#include "RF_TXTask.hpp"
#include "Logger.hpp"

using namespace AT86RF215;

void RF_RXTask::ensureRxMode() {
    if (transceiver.get_state(RF09, error) != State::RF_RX) {
        transceiver.set_state(RF09, RF_TRXOFF, error);
        vTaskDelay(10);
        transceiver.set_state(RF09, RF_TXPREP, error);
        vTaskDelay(20);
        transceiver.set_state(RF09, State::RF_RX, error);
    }
}
void RF_RXTask::execute() {
    vTaskDelay(5000);
    LOG_INFO << "[RF RX TASK]";
    /// Set the Up-link frequency
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    /// ENABLE THE RX SWITCH
    HAL_GPIO_WritePin(EN_RX_UHF_GPIO_Port, EN_RX_UHF_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_RESET);
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
        } else
            LOG_ERROR << "CONNECTION ##ERROR## WITH CODE: " << status.error();
        transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
        transceiver.setup(error);
        xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
    }
    transceiver.set_state(RF09, RF_TXPREP, error);
    vTaskDelay(pdMS_TO_TICKS(20));
    transceiver.set_state(RF09, RF_RX, error);
    if (transceiver.get_state(RF09, error) == RF_RX)
        LOG_INFO << "[RF RX TASK] INITIAL STATE = RX";
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_SET);
    /// Reference time for TX notification
    TickType_t lastTxNotifyTime = xTaskGetTickCount();
    xTaskNotify(rf_txtask->taskHandle, START_TX_TASK, eSetBits);
    uint32_t receivedEvents = 0;
    uint16_t received_length = 0;
    while (1) {
        /// Notify TX task
        if ((xTaskGetTickCount() - lastTxNotifyTime) >= pdMS_TO_TICKS(2000)) {
            xTaskNotify(rf_txtask->taskHandle, TRANSMIT, eSetBits);
            lastTxNotifyTime = xTaskGetTickCount();
        }
        if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
            ensureRxMode();
            xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
        }
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &receivedEvents, pdMS_TO_TICKS(50))) {
            if (receivedEvents & RXFE) {
                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    transceiver.print_error(error);
                    if (result.has_value()) {
                        received_length = result.value();
                        LOG_INFO << "RX PACKET WITH RECEPTION LENGTH: " << received_length;
                        LOG_INFO << "Counter packet: " << transceiver.spi_read_8((AT86RF215::BBC0_FBRXS), error);
                        transceiver.print_error(error);
                    } else {
                        Error err = result.error();
                        LOG_ERROR << "AT86RF215 ##ERROR## AT RX LENGTH RECEPTION WITH CODE: " << err;
                    }
                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                }
            }
            if (receivedEvents & AGC_HOLD) {
                LOG_INFO << "RSSI [AGC HOLD]: " << transceiver.get_rssi(RF09, error);
                transceiver.print_error(error);
            }
        }
    }
}