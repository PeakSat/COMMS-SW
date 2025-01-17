#include "RF_TXTask.hpp"
#include "Logger.hpp"
#include <timers.h>
#include "main.h"
#include "app_main.h"

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
            // transceiver.set_state(RF09, RF_TRXOFF, error);
        break;
        case RF_RX:
            LOG_DEBUG << "[TX ENSURE] STATE: RX";
            if (transceiver.rx_ongoing == false && transceiver.agc_held == false)
                // transceiver.set_state(RF09, RF_TRXOFF, error);
                    __NOP();
            else {
                LOG_DEBUG << "[TX ENSURE] RX_ONGOING " << transceiver.rx_ongoing;
                LOG_DEBUG << "[TX ENSURE] AGC HELD " << transceiver.agc_held;
            }
            break;
        case RF_TRANSITION:
            LOG_DEBUG << "[TX ENSURE] STATE: TRANSITION";
        break;
        case RF_RESET:
            LOG_DEBUG << "[TX ENSURE] STATE: RESET";
        break;
        case RF_INVALID:
            LOG_DEBUG << "[TX ENSURE] STATE: INVALID";
        break;
        case RF_TXPREP:
            LOG_DEBUG << "[TX ENSURE] STATE: TXPREP";
            // transceiver.set_state(RF09, RF_TRXOFF, error);
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
    uint32_t receivedEvents = 0;
    if (xTaskNotifyWait(START_TX_TASK, START_TX_TASK, &receivedEvents, portMAX_DELAY) == pdTRUE) {
        if (receivedEvents & START_TX_TASK) {
            LOG_INFO << "Starting RF TX Task NOMINALLY"; // Handle the event
        }
    } else {
        // In case the notification was not received, you can log or handle it
        LOG_ERROR << "RF RX Task notification error.";
    }
    LOG_INFO << "Starting RF TX Task"; // Handle the event
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
        transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
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
    TimerHandle_t xTimer = xTimerCreate("Transmit Timer", pdMS_TO_TICKS(500), pdTRUE, (void *)nullptr, vTimerCallback);
    if (xTimer != NULL) {
        if (xTimerStart(xTimer, 0) != pdPASS) {
            LOG_ERROR << "[TX TASK] Failed to start the timer";
        }
    } else {
        LOG_INFO << "[TX TASK] START THE TX TIMER";
    }

    uint8_t transmit_notify_counter = 0;
    uint8_t transmit = 0;
    while (1) {
        /// without delay there is an issue on the receiver. The TX stops the transmission after a while and the receiver loses packets.
        /// But why ? There is also the delay of the TXFE interrupt which is around [1 / (50000 / (packetLenghtInbits)) s]. It should work just fine...
        /// Maybe there is an error either on the tx side or the rx side, which is not printed.
        /// Typically 200ms delay works.
        /// We had the same issue on the campaign.
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &receivedEvents, portMAX_DELAY) == pdTRUE) {
            if (receivedEvents & TRANSMIT) {
                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                    // Prepare and send the packet
                    if (transceiver.rx_ongoing){
                        xTimerStop(xTimer, 0);
                        if (xTaskNotifyWait(RXFE, RXFE, &receivedEvents, pdTICKS_TO_MS(500)) == pdTRUE) {
                            if (receivedEvents & RXFE) {
                                xTimerStart(xTimer, 0);
                                LOG_DEBUG << "[TX TASK] RECEIVE FRAME END";
                                transmit = 1;
                            }
                        }
                        else {
                            xTimerStart(xTimer, 0);
                            transmit = 1;
                            LOG_ERROR << "[TX TASK] RECEIVE FRAME END DID NOT RECEIVED - TIMER STARTED ANYWAY";
                        }
                    }
                    else if (transceiver.tx_ongoing) {
                        xTimerStop(xTimer, 0);
                        if (xTaskNotifyWait(TXFE, TXFE, &receivedEvents, pdTICKS_TO_MS(500)) == pdTRUE) {
                            if (receivedEvents & TXFE) {
                                xTimerStart(xTimer, 0);
                                transmit = 1;
                                LOG_DEBUG << "[TX TASK] TRANSMIT FRAME END";
                            }
                        }
                        else {
                            xTimerStart(xTimer, 0);
                            transmit = 1;
                            LOG_ERROR << "[TX TASK] TRANSMIT FRAME END DID NOT RECEIVED - TIMER STARTED ANYWAY";
                        }
                    }
                    else
                        LOG_INFO << "[TX TASK] - NOMINAL";
                    if (transceiver.get_state(RF09, error) != RF_TRANSITION && transmit) {
                            packetTestData.packet[0] = counter++;
                            transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                            transceiver.print_error(error);
                            LOG_INFO << "[TX TASK] Transmitted packet: " << counter;
                            LOG_INFO << "[TX TASK] LENGTH: " << packetTestData.length;
                            transmit = 0;
                    }
                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                }
                transmit_notify_counter++;
                if (transmit_notify_counter == 255)
                    transmit_notify_counter = 0;
                if (counter == 255)
                    counter = 0;
            }
        }
    }
}