#include "RF_TXTask.hpp"
#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <timers.h>
#include "main.h"
#include "app_main.h"

void RF_TXTask::ensureTxMode() {
    State state = transceiver.get_state(RF09, error);
    switch (state) {
        case RF_NOP:
            LOG_DEBUG << "[TX ENSURE] STATE: NOP";
            transceiver.set_state(RF09, RF_TRXOFF, error);
        break;
        case RF_SLEEP:
            LOG_DEBUG << "[TX ENSURE] STATE: SLEEP";
            transceiver.set_state(RF09, RF_TRXOFF, error);
        break;
        case RF_TRXOFF:
            LOG_DEBUG << "[TX ENSURE] STATE: TRXOFF";
        break;
        case RF_TX:
            LOG_DEBUG << "[TX ENSURE] STATE: TX";
            transceiver.set_state(RF09, RF_TRXOFF, error);
        break;
        case RF_RX:
            transceiver.set_state(RF09, RF_TRXOFF, error);
            // LOG_DEBUG << "[TX ENSURE] STATE: RX";
            break;
        case RF_TRANSITION:
            vTaskDelay(10);
            LOG_DEBUG << "[TX ENSURE] STATE: TRANSITION";
        break;
        case RF_RESET:
            LOG_DEBUG << "[TX ENSURE] STATE: RESET";
        break;
        case RF_INVALID:
            LOG_DEBUG << "[TX ENSURE] STATE: INVALID";
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
            vTaskDelay(20);
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
            vTaskDelay(10);
            transceiver.set_state(RF09, RF_TRXOFF, error);
        break;
        case RF_TXPREP:
            // LOG_DEBUG << "[TX ENSURE] STATE: TXPREP";
            transceiver.set_state(RF09, RF_TRXOFF, error);
        break;
        default:
            LOG_ERROR << "UNDEFINED";
        break;
    }
}

PacketData RF_TXTask::createRandomPacketData(uint16_t length) {
    PacketData data{};
    for (uint16_t i = 0; i < length; i++)
        data.packet[i] = i % 100;
    data.length = length;
    return data;
}

[[noreturn]] void RF_TXTask::execute() {
    vTaskDelay(8000);
    /// TX AMP
    GPIO_PinState txamp = GPIO_PIN_SET;
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
    if (txamp)
        LOG_DEBUG << "##########TX AMP DISABLED##########";
    else
        LOG_DEBUG << "##########TX AMP ENABLED##########";
    PacketData packetTestData = createRandomPacketData(MaxPacketLength);
    StaticTimer_t xTimerBuffer;
    TimerHandle_t xTimer = xTimerCreateStatic(
        "Transmit Timer",
        pdMS_TO_TICKS(TX_TRANSMIT),
        pdTRUE,
        (void *)1,
        [](TimerHandle_t pxTimer) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyIndexedFromISR(
                rf_txtask->taskHandle,
                NOTIFY_INDEX_TRANSMIT,
                TRANSMIT,
                eSetBits,
                &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        },
        &xTimerBuffer);

    if (xTimer != nullptr) {
        if (xTimerStart(xTimer, 0) != pdPASS) {
            LOG_ERROR << "[TX] Failed to start the timer";
        }
        else
            LOG_INFO << "[TX] TX TIMER HAS STARTED";
    }
    uint8_t state = 0;
    uint8_t counter = 0;
    uint32_t receivedEventsTransmit;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TRANSMIT, pdFALSE, pdTRUE, &receivedEventsTransmit, pdTICKS_TO_MS(2000)) == pdTRUE) {
            if (receivedEventsTransmit & TRANSMIT) {
                if (counter == 255)
                    counter = 0;
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing;
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
                switch (state) {
                    case READY: {
                        if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                            if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                ensureTxMode();
                                /// Set the down-link frequency
                                transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
                                transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                                transceiver.chip_reset(error);
                                transceiver.setup(error);
                                /// send the packet
                                counter++;
                                packetTestData.packet[0] = counter;
                                transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                LOG_INFO << "[TX] c: " << counter;
                            }
                            xSemaphoreGive(transceiver_handler.resources_mtx);
                        }
                        break;
                    }
                    case TX_ONG: {
                        uint32_t receivedEventsTXFE;
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TXFE, pdFALSE, pdTRUE, &receivedEventsTXFE, pdTICKS_TO_MS(1000)) == pdTRUE) {
                            if (receivedEventsTXFE & TXFE) {
                                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                                    if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                        ensureTxMode();
                                        /// Set the down-link frequency
                                        transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
                                        transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                                        transceiver.chip_reset(error);
                                        transceiver.setup(error);
                                        /// send the packet
                                        counter++;
                                        packetTestData.packet[0] = counter;
                                        transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                        LOG_INFO << "[TX TXFE] c: " << counter;
                                    }
                                    xSemaphoreGive(transceiver_handler.resources_mtx);
                                }
                            }
                        }
                        break;
                    }
                    case RX_ONG: {
                        uint32_t receivedEventsRXFE;
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_AGC_RELEASE, pdFALSE, pdTRUE, &receivedEventsRXFE, pdTICKS_TO_MS(1000)) == pdTRUE) {
                            if (receivedEventsRXFE & AGC_RELEASE) {
                                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                                    if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                        ensureTxMode();
                                        /// Set the down-link frequency
                                        transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
                                        transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                                        transceiver.chip_reset(error);
                                        transceiver.setup(error);
                                        // send the packet
                                        counter++;
                                        packetTestData.packet[0] = counter;
                                        transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                        LOG_INFO << "[TX RXFE] c: " << counter;
                                    }
                                    xSemaphoreGive(transceiver_handler.resources_mtx);
                                }
                            }
                        }
                        break;
                    }
                    case RX_TX_ONG: {
                        LOG_ERROR << "[TX] RXONG TXONG";
                        break;
                    }
                    default: {
                        LOG_ERROR << "[TX] Unknown state!";
                        break;
                    }
                }
            }
        }else {
            LOG_ERROR << "[TX] Failed to get the event from the timer";
        }
    }
}
