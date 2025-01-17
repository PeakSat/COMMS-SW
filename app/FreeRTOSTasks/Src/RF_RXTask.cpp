#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <RF_TXTask.hpp>

using namespace AT86RF215;

void RF_RXTask::ensureRxMode() {
    State rf_state = transceiver.get_state(RF09, error);
    switch (rf_state) {
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
                transceiver.print_state(RF09, error);
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
                vTaskDelay(10);
                transceiver.set_state(RF09, RF_RX, error);
                transceiver.print_state(RF09, error);
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
    xTaskNotify(rf_txtask->taskHandle, START_TX_TASK, eSetBits);
    uint32_t receivedEvents = 0;
    uint16_t received_length = 0;
    while (1) {
        if (transceiver.rx_ongoing == false && transceiver.tx_ongoing == false) {
            if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                ensureRxMode();
                xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
            }
        }
        if (xTaskNotifyWait(0, RXFE, &receivedEvents, pdMS_TO_TICKS(50))) {
            if (receivedEvents & RXFE || receivedEvents & RXFS || receivedEvents & AGC_HOLD ) {
                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    received_length = result.value();
                    if (received_length == 1024) {
                        // LOG_INFO << "RX PACKET WITH RECEPTION LENGTH: " << received_length;
                        LOG_INFO << "RX Counter packet: " << transceiver.spi_read_8((BBC0_FBRXS), error);
                        // ensureRxMode();
                    }
                    else {
                        transceiver.print_error(error);
                        LOG_ERROR << "DROP RX MESSAGE";
                        // ensureRxMode();
                    }
                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                }
            }
        }
    }
}

