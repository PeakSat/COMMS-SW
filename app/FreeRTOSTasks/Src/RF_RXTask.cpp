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
    xTaskNotify(rf_txtask->taskHandle, START_TX_TASK, eSetBits);
    uint16_t received_length;
    while (1) {
        uint32_t receivedEvents = 0;
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &receivedEvents, pdMS_TO_TICKS(1000))) {
            if (receivedEvents & RXFE) {
                LOG_DEBUG << "RECEIVE FRAME START";
            }
            if (receivedEvents & RXFE) {
                LOG_DEBUG << "RECEIVE FRAME END";
                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    if (result.has_value()) {
                        received_length = result.value();
                        LOG_INFO << "RX PACKET WITH RECEPTION LENGTH: " << received_length;
                    } else {
                        Error err = result.error();
                        LOG_ERROR << "AT86RF215 ##ERROR## AT RX LENGTH RECEPTION WITH CODE: " << err;
                    }
                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                }
            }
            if (receivedEvents & TRXRDY)
                LOG_DEBUG << "TRANSCEIVER IS READY.";
            if (receivedEvents & TRXERR)
                LOG_ERROR << "PLL UNLOCKED.";
            if (receivedEvents & IFSERR)
                LOG_ERROR << "SYNCHRONIZATION ERROR.";
        }
        if (xTaskNotifyWait(AGC_HOLD, AGC_HOLD, &receivedEvents, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (receivedEvents & AGC_HOLD) {
                /// TODO: HANDLE THE RECEIVED PACKET
                LOG_DEBUG << "AGC HOLD NOTIFICATION";
                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    if (result.has_value()) {
                        received_length = result.value();
                        LOG_INFO << "RX PACKET WITH RECEPTION LENGTH: " << received_length;
                    } else {
                        /// TODO: Error handling
                        Error err = result.error();
                        LOG_ERROR << "AT86RF215 ##ERROR## AT RX LENGTH RECEPTION WITH CODE: " << err;
                    }
                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                }
            }
        }
    }
}
