#include "RF_TXTask.hpp"
#include "RF_RXTask.hpp"

#include "Logger.hpp"
#include <timers.h>
#include "main.h"
#include "app_main.h"

#include <RF_RXTask.hpp>

void RF_TXTask::ensureTxMode() {
    State rf_state = transceiver.get_state(RF09, error);
    switch (rf_state) {
        case RF_NOP:
            LOG_DEBUG << "[TX ENSURE] STATE: NOP";
        break;
        case RF_SLEEP:
            LOG_DEBUG << "[TX ENSURE] STATE: SLEEP";
        break;
        case RF_TRXOFF:
            LOG_DEBUG << "[TX ENSURE] STATE: TRXOFF";
        break;
        case RF_TX:
            LOG_DEBUG << "[TX ENSURE] STATE: TX";
        break;
        case RF_RX:
            // transceiver.set_state(RF09, RF_TRXOFF, error);
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

void RF_TXTask::execute() {

    vTaskDelay(8000);
    // if (xTaskNotifyWaitIndexed(4, 0, START_TX_TASK, &receivedEvents, portMAX_DELAY) == pdTRUE) {
    //     if (receivedEvents & START_TX_TASK) {
    //         LOG_INFO << "Starting RF TX Task NOMINALLY"; // Handle the event
    //     }
    // } else {
    //     // In case the notification was not received, you can log or handle it
    //     LOG_ERROR << "RF RX Task notification error.";
    // }
    // LOG_INFO << "Starting RF TX Task"; // Handle the event
    /// Check transceiver connection
    if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
        auto status = transceiver.check_transceiver_connection(error);
        if (status.has_value()) {
            LOG_INFO << "AT86RF215 CONNECTION FROM TX TASK OK";
        } else
            /// TODO: Error handling
                LOG_ERROR << "AT86RF215 ##ERROR## WITH CODE: " << status.error();
        /// Set the down-link frequency
        transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
        transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
        transceiver.chip_reset(error);
        transceiver.setup(error);
        xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
    }
    /// TX AMP
    GPIO_PinState txamp = GPIO_PIN_SET;
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
    if (txamp)
        LOG_DEBUG << "##########TX AMP DISABLED##########";
    else
        LOG_DEBUG << "##########TX AMP ENABLED##########";
    uint8_t counter = 0;
    PacketData packetTestData = createRandomPacketData(MaxPacketLength);
    StaticTimer_t xTimerBuffer;
    TimerHandle_t xTimer =  xTimerCreateStatic("Transmit Timer", pdMS_TO_TICKS(2000), pdTRUE, (void *)1, vTimerCallback, &xTimerBuffer);
    if (xTimer != nullptr) {
        if (xTimerStart(xTimer, 0) != pdPASS) {
            LOG_ERROR << "[TX TASK] Failed to start the timer";
        }
    } else {
        LOG_INFO << "[TX TASK] START THE TX TIMER";
    }
    UBaseType_t uxHighWaterMark;
    uint32_t receivedEventsTransmit;
    uint8_t state = 3;
    while (1) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TRANSMIT, pdFALSE, pdTRUE, &receivedEventsTransmit, portMAX_DELAY) == pdTRUE) {
            if (receivedEventsTransmit & TRANSMIT) {
            if (counter == 255)
                counter = 0;
            if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing;
                xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
            }
            switch (state) {
                case 0: { // rx_ongoing = false, tx_ongoing = false
                    // LOG_INFO << "[TX TASK] NOM";
                    if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                        // LOG_DEBUG << "[TX TASK] nominal : took the semaphore";
                        if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                            ensureTxMode();
                            counter++;
                            packetTestData.packet[0] = counter;
                            transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                            LOG_INFO << "[TX NOM] packet: " << counter;
                        }
                        xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                    } else {
                        LOG_DEBUG << "[TX TASK] - COULD NOT ACQUIRE THE SEMAPHORE";
                    }
                    break;
                }

                case 1: { // rx_ongoing = false, tx_ongoing = true
                    uint32_t receivedEventsTXFE = 0;
                    if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TXFE, pdFALSE, pdTRUE, &receivedEventsTXFE, portMAX_DELAY) == pdTRUE) {
                        if (receivedEventsTXFE & TXFE) {
                            // LOG_DEBUG << "[TX TASK] TXFE" ;
                            if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                                // LOG_DEBUG << "[TX TASK] TXFE : took the semaphore";
                                if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                    ensureTxMode();
                                    counter++;
                                    packetTestData.packet[0] = counter;
                                    transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                    transceiver.print_error(error);
                                    LOG_INFO << "[TX TXFE] packet: " << counter;
                                }
                                xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                            } else
                                LOG_DEBUG << "[TX TASK] - COULD NOT ACQUIRE THE SEMAPHORE";

                        } else {
                            LOG_DEBUG << "[TX TASK] TXFE - OTHER EVENT";
                        }
                    }
                    break;
                }
                case 2: { // rx_ongoing = true, tx_ongoing = false
                    uint32_t receivedEventsRXFE;
                    if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RXFE_TX, pdFALSE, pdTRUE, &receivedEventsRXFE, portMAX_DELAY) == pdTRUE) {
                        if (receivedEventsRXFE & RXFE_TX) {
                            if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                                if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                    ensureTxMode();
                                    counter++;
                                    packetTestData.packet[0] = counter;
                                    transceiver.print_error(error);
                                    transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                    transceiver.print_error(error);
                                    LOG_INFO << "[TX TASK - RXFE] packet: " << counter;
                                }
                                xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                            } else {
                                LOG_DEBUG << "[TX TASK RXFE] - COULD NOT ACQUIRE THE SEMAPHORE";
                            }
                        } else {
                            LOG_DEBUG << "[TX TASK] RXFE - OTHER EVENT";
                        }
                    }
                    break;
                }

                case 3: { // rx_ongoing = true, tx_ongoing = true
                    // Potential deadlock
                    LOG_ERROR << "[TX TASK] CASE 3";
                    break;
                }
                default: {
                    LOG_ERROR << "[TX TASK] Unknown state!";
                    break;
                }
            }
        }
            }
    }
}
