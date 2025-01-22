#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <RF_TXTask.hpp>

using namespace AT86RF215;

void RF_RXTask::ensureRxMode() {
    State trx_state = transceiver.get_state(RF09, error);
    switch (trx_state) {
        case RF_NOP:
                LOG_DEBUG << "[RX ENSURE] STATE: NOP";
        break;
        case RF_SLEEP:
                LOG_DEBUG << "[RX ENSURE] STATE: SLEEP";
        break;
        case RF_TRXOFF:
                LOG_DEBUG << "[RX ENSURE] STATE: TRXOFF";
                transceiver.set_state(RF09, RF_TXPREP, error);
                /// the delay here is essential
                vTaskDelay(20);
                transceiver.set_state(RF09, RF_RX, error);
        break;
        case RF_TX:
                LOG_DEBUG << "[RX ENSURE] STATE: TX";
                HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
                vTaskDelay(20);
                HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
                transceiver.set_state(RF09, RF_TXPREP, error);
                /// the delay here is essential
                vTaskDelay(20);
                transceiver.set_state(RF09, RF_RX, error);
                transceiver.print_state(RF09, error);
        break;
        case RF_RX:
            LOG_DEBUG << "[RX ENSURE] STATE: RX";
        break;
        case RF_TRANSITION:
                LOG_DEBUG << "[RX ENSURE] STATE: TRANSITION";
        break;
        case RF_RESET:
                LOG_DEBUG << "[RX ENSURE] STATE: RESET";
        break;
        case RF_INVALID:
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
            vTaskDelay(20);
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
            vTaskDelay(10);
            transceiver.set_state(RF09, RF_TRXOFF, error);
            vTaskDelay(10);
            transceiver.set_state(RF09, RF_TXPREP, error);
            /// the delay here is essential
            vTaskDelay(20);
            transceiver.set_state(RF09, RF_RX, error);
            LOG_DEBUG << "[RX ENSURE] STATE: INVALID";
            transceiver.print_state(RF09, error);
        break;
        case RF_TXPREP:
                LOG_DEBUG << "[RX ENSURE] STATE: TXPREP";
                transceiver.set_state(RF09, RF_RX, error);
        break;
        default:
            LOG_ERROR << "UNDEFINED";
        break;
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
    if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
        auto status = transceiver.check_transceiver_connection(error);
        if (status.has_value()) {
        } else
            LOG_ERROR << "CONNECTION ##ERROR## WITH CODE: " << status.error();
        transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
        transceiver.setup(error);
        xSemaphoreGive(transceiver_handler.resources_mtx);
    }
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_SET);
    uint16_t received_length = 0;
    uint8_t current_counter = 0;
    uint32_t drop_counter = 0;
    uint32_t print_tx_ong = 0;
    uint32_t receivedEvents;
    uint8_t rf_state;
    State trx_state;
    LOG_DEBUG << "tx_ong" << transceiver.tx_ongoing;
    LOG_DEBUG << "rx_ong" << transceiver.rx_ongoing;

    while (1) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_AGC, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(RX_REFRESH_PERIOD_MS)) == pdTRUE) {
            if (receivedEvents & AGC_HOLD) {
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    received_length = result.value();
                    if (received_length == 1024) {
                        current_counter = transceiver.spi_read_8((BBC0_FBRXS), error);
                        LOG_DEBUG << "[RF RX] c: " << current_counter;
                        drop_counter = 0;
                    }
                    else {
                        drop_counter++;
                        LOG_DEBUG << "[RF RX](DROP) c: " << drop_counter;
                    }
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
            }
        }
        else {
            if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                switch (rf_state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing) {
                    case 0: { // rx_ongoing = false, tx_ongoing = false
                         trx_state = transceiver.get_state(RF09, error);
                        if (trx_state != RF_RX)
                            ensureRxMode();
                        break;
                    }
                    case 1: { // rx_ongoing = false, tx_ongoing = true
                        // Handle the case where tx_ongoing is true
                        // xTaskNotifyGive(rf_txtask->taskHandle);
                        // LOG_DEBUG << "[RX TASK] TXONG";
                        break;
                    }
                    case 2: { // rx_ongoing = true, tx_ongoing = false
                        // Handle the case where rx_ongoing is true
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RXFE_RX, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(500))) {
                            if (receivedEvents &  RXFE_RX) {
                                trx_state = transceiver.get_state(RF09, error);
                                if (trx_state != RF_RX)
                                    ensureRxMode();
                            }
                        }
                        trx_state = transceiver.get_state(RF09, error);
                        if (trx_state != RF_RX)
                            ensureRxMode();
                        break;
                    }
                    case 3: { // rx_ongoing = true, tx_ongoing = true
                        // Handle the case where both are true
                        LOG_DEBUG << "[RX TASK] RXONG AND TXONG";
                        break;
                    }
                    default: {
                        // Handle any unexpected cases
                        break;
                    }
                }
                xSemaphoreGive(transceiver_handler.resources_mtx);
            }
        }

    }
}
