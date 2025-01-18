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
            transceiver.set_state(RF09, RF_TRXOFF, error);
            LOG_DEBUG << "[TX ENSURE] STATE: RX";
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
            LOG_DEBUG << "[TX ENSURE] STATE: TXPREP";
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
    TimerHandle_t xTimer =  xTimerCreateStatic("Transmit Timer", pdMS_TO_TICKS(500), pdTRUE, (void *)1, vTimerCallback, &xTimerBuffer);
    if (xTimer != NULL) {
        if (xTimerStart(xTimer, 0) != pdPASS) {
            LOG_ERROR << "[TX TASK] Failed to start the timer";
        }
    } else {
        LOG_INFO << "[TX TASK] START THE TX TIMER";
    }
    UBaseType_t uxHighWaterMark;
    while (1) {
        // wait for index 0
        uint32_t receivedEventsTransmit = 0;
        // wait for index 0
            if (xTaskNotifyWaitIndexed(0, 0, TRANSMIT, &receivedEventsTransmit, pdMS_TO_TICKS(5000)) == pdTRUE) {
                if (receivedEventsTransmit & TRANSMIT) {
                    uxHighWaterMark =  uxTaskGetStackHighWaterMark(this->taskHandle);
                    LOG_DEBUG << "[TX TASK] remaining stack: " << uxHighWaterMark;
                    receivedEventsTransmit = 0;
                    // Prepare and send the packet
                    transceiver.print_error(error);
                    if(transceiver.tx_ongoing == false && transceiver.rx_ongoing == false) {
                        LOG_INFO << "[TX TASK] NOMINAL" ;
                         if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
                             if (transceiver.rx_ongoing == false && transceiver.tx_ongoing == false) {
                                 // vTaskSuspendAll();
                                 ensureTxMode();
                                 transceiver.print_error(error);
                                 packetTestData.packet[0] = counter++;transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                 transceiver.print_error(error);
                                 // xTaskResumeAll();
                                 LOG_INFO << "[TX TASK - NOMINAL] Transmitted packet: " << counter;
                             }
                             xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                         }else {
                            LOG_DEBUG << "[TX TASK] - COULD NOT ACQUIRE THE SEMAPHORE";
                        }
                    }
                    else if (transceiver.rx_ongoing){
                        // wait for index 2
                        uint32_t receivedEventsRXFE;
                        if (xTaskNotifyWaitIndexed(2, 0, RXFE, &receivedEventsRXFE, pdTICKS_TO_MS(1000)) == pdTRUE){
                            if (receivedEventsRXFE & RXFE) {
                                LOG_DEBUG << "[TX TASK]: " << transceiver.tx_ongoing;
                                LOG_DEBUG << "[TX TASK]: " << transceiver.rx_ongoing;
                                LOG_DEBUG << "[TX TASK] RECEIVE FRAME END";
                                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
                                    if (transceiver.tx_ongoing == false && transceiver.rx_ongoing == false) {
                                        packetTestData.packet[0] = counter++;
                                        transceiver.print_error(error);
                                        transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                        transceiver.print_error(error);
                                        LOG_INFO << "[TX TASK - RXFE] Transmitted packet: " << counter;
                                    }
                                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                                }else {
                                    LOG_DEBUG << "[TX TASK RXFE] - COULD NOT ACQUIRE THE SEMAPHORE";
                                }
                            }
                            else {
                                transceiver.print_error(error);
                                LOG_DEBUG << "[TX TASK] RXFE - OTHER EVENT" << receivedEventsRXFE;
                            }
                        }else {
                            transceiver.print_error(error);
                            LOG_DEBUG << "[TX TASK]: " << transceiver.tx_ongoing;
                            LOG_DEBUG << "[TX TASK]: " << transceiver.rx_ongoing;

                        }
                    }
                    else if (transceiver.tx_ongoing) {
                        // wait for index 1
                        uint32_t receivedEventsTXFE;
                        if (xTaskNotifyWaitIndexed(1, 0, TXFE, &receivedEventsTXFE, pdTICKS_TO_MS(1000)) == pdTRUE) {
                            if (receivedEventsTXFE & TXFE) {
                                LOG_DEBUG << "[TX TASK] TRANSMIT FRAME END";
                                transceiver.print_error(error);
                                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
                                    if (transceiver.tx_ongoing == false && transceiver.rx_ongoing == false) {
                                        packetTestData.packet[0] = counter++;
                                        transceiver.print_error(error);
                                        transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                        transceiver.print_error(error);
                                        LOG_INFO << "[TX TASK - RXFE] Transmitted packet: " << counter;
                                    }
                                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                                }else {
                                    LOG_DEBUG << "[TX TASK] - COULD NOT ACQUIRE THE SEMAPHORE";
                                }
                            }
                            else {
                                LOG_DEBUG << "[TX TASK] TXFE - OTHER EVENT";
                                transceiver.print_error(error);
                            }
                        }
                        else {
                            // xTimerStart(xTimer, 0);
                            transceiver.print_error(error);
                            LOG_ERROR << "[TX TASK - TXFE]: " << receivedEventsTXFE;
                            LOG_ERROR << "[TX TASK] TRANSMIT FRAME END DID NOT RECEIVED - TIMER STARTED ANYWAY";
                        }
                    }
                    else {
                        LOG_DEBUG << "[TX TASK]: " << transceiver.tx_ongoing;
                        LOG_DEBUG << "[TX TASK]: " << transceiver.rx_ongoing;
                    }
                }
            }
    }
}
