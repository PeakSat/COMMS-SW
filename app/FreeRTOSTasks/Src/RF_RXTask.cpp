#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <RF_TXTask.hpp>

using namespace AT86RF215;

void RF_RXTask::ensureRxMode() {
    switch (State trx_state = transceiver.get_state(RF09, error)) {
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
            // transceiver.print_state(RF09, error);
            break;
        case RF_RX:
            // LOG_DEBUG << "[RX ENSURE] STATE: RX";
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
            LOG_ERROR << "[RX ENSURE] STATE: UNDEFINED";
            break;
    }
}

[[noreturn]] void RF_RXTask::execute() {
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
        transceiver.set_state(RF09, RF_TRXOFF, error);
        transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
        transceiver.chip_reset(error);
        xSemaphoreGive(transceiver_handler.resources_mtx);
    }
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_SET);
    uint16_t received_length = 0;
    uint8_t current_counter = 0;
    uint32_t drop_counter = 0;
    uint32_t print_tx_ong = 0;
    uint32_t receivedEvents;
    State trx_state;
    ensureRxMode();
    uint32_t rx_total_packets = 0;
    uint32_t rx_total_drop_packets = 0;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_AGC, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(transceiver_handler.RX_REFRESH_PERIOD_MS)) == pdTRUE) {
            if (receivedEvents & AGC_HOLD) {
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    received_length = result.value();
                    if (received_length == 1024) {
                        current_counter = transceiver.spi_read_8((BBC0_FBRXS), error);
                        LOG_DEBUG << "[RX] c: " << current_counter;
                        rx_total_packets++;
                        LOG_DEBUG << "[RX] total packets c: " << rx_total_packets;
                        drop_counter = 0;
                    }
                    else {
                        drop_counter++;
                        rx_total_drop_packets++;
                        LOG_DEBUG << "[RX DROP] c: " << drop_counter;
                        LOG_DEBUG << "[RX DROP] total packets c: " << rx_total_drop_packets;
                    }
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
            }
        }
        else {
            if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                switch (uint8_t rf_state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing) {
                    case READY: {
                        trx_state = transceiver.get_state(RF09, error);
                        if (trx_state != RF_RX) {
                            // set the uplink frequency
                            // transceiver.set_state(RF09, RF_TRXOFF, error);
                            // transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
                            // transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                            // transceiver.chip_reset(error);
                            // transceiver.setup(error);
                            ensureRxMode();
                        }
                        break;
                    }
                    case TX_ONG: {
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TXFE_RX, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(1000)) == pdTRUE) {
                            if (receivedEvents & TXFE) {
                                ensureRxMode();
                                LOG_INFO << "[RX] TXFE";
                            }
                        }
                        else {
                            LOG_ERROR << "[RX] TXFE NOT RECEIVED";
                            // DEADLOCK HANDLING
                            transceiver.print_error(error);
                            transceiver.print_state(RF09, error);
                            transceiver.set_state(RF09, RF_TRXOFF, error);
                            transceiver.chip_reset(error);
                            transceiver.print_error(error);
                            transceiver.tx_ongoing = false;
                            ensureRxMode();
                        }
                        break;
                    }
                    case RX_ONG: {
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RXFE_RX, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(1000))) {
                            if (receivedEvents &  RXFE_RX) {
                                trx_state = transceiver.get_state(RF09, error);
                                if (trx_state != RF_RX) {
                                    // set the uplink frequency
                                    // transceiver.set_state(RF09, RF_TRXOFF, error);
                                    // transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
                                    // transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                                    // transceiver.chip_reset(error);
                                    // transceiver.setup(error);
                                    ensureRxMode();
                                }
                            }
                        }
                        trx_state = transceiver.get_state(RF09, error);
                        if (trx_state != RF_RX) {
                            // set the uplink frequency
                            // transceiver.set_state(RF09, RF_TRXOFF, error);
                            // transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
                            // transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                            // transceiver.chip_reset(error);
                            // transceiver.setup(error);
                            ensureRxMode();
                        }
                        break;
                    }
                    case RX_TX_ONG: {
                        LOG_DEBUG << "[RX] RXONG AND TXONG";
                        break;
                    }
                    default: {
                        LOG_ERROR << "[RX] UNEXPECTED CASE";
                        break;
                    }
                }
            }
            if (transceiver.TransceiverError_flag) {
                transceiver.TransceiverError_flag = false;
                LOG_ERROR << "[RX] Transceiver Error";
            }
            if (transceiver.FrameBufferLevelIndication_flag) {
                transceiver.FrameBufferLevelIndication_flag = false;
                LOG_ERROR << "[RX] FrameBuffer Level Indication";
            }
            if (transceiver.IFSynchronization_flag) {
                transceiver.IFSynchronization_flag = false;
                LOG_ERROR << "[RX] IF Synchronization";
            }
            if (transceiver.Voltage_Drop) {
                transceiver.Voltage_Drop = false;
                LOG_ERROR << "[RX] Voltage Drop";
            }
            xSemaphoreGive(transceiver_handler.resources_mtx);
        }
    }
}
