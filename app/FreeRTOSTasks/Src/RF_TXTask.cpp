#include "RF_TXTask.hpp"
#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <timers.h>
#include "main.h"
#include "app_main.h"

#include <ApplicationLayer.hpp>
#include <eMMC.hpp>

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
    outgoingTMQueue = xQueueCreateStatic(TMQueueSize, sizeof(CAN::StoredPacket), outgoingTMQueueStorageArea,
                                            &outgoingTMQueueBuffer);
    vQueueAddToRegistry(outgoingTMQueue, "TM queue");
    vTaskDelay(8000);
    PacketData packetTestData = createRandomPacketData(MaxPacketLength);
    StaticTimer_t xTimerBuffer;
    TimerHandle_t xTimer = xTimerCreateStatic(
        "Transmit Timer",
        pdMS_TO_TICKS(transceiver_handler.BEACON_PERIOD_MS),
        pdTRUE,
        (void *)1,
        [](TimerHandle_t pxTimer) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            // xTaskNotifyIndexedFromISR(
            //     rf_txtask->taskHandle,
            //     NOTIFY_INDEX_TRANSMIT,
            //     TRANSMIT,
            //     eSetBits,
            //     &xHigherPriorityTaskWoken);
            // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        },
        &xTimerBuffer);

    // if (xTimer != nullptr) {
    //     if (xTimerStart(xTimer, 0) != pdPASS) {
    //         LOG_ERROR << "[TX] Failed to start the timer";
    //     }
    //     else
    //         LOG_INFO << "[TX] TX TIMER HAS STARTED";
    // }
    uint8_t state = 0;
    uint8_t counter = 0;
    uint32_t receivedEventsTransmit;
    uint32_t tx_counter = 0;
    CAN::StoredPacket TM_PACKET;
    /// TX AMP
    GPIO_PinState txamp;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TRANSMIT, pdFALSE, pdTRUE, &receivedEventsTransmit, pdTICKS_TO_MS(transceiver_handler.BEACON_PERIOD_MS + 1000)) == pdTRUE) {
            while (uxQueueMessagesWaiting(outgoingTMQueue)) {
                // Get the message pointer from the queue
                /// TODO: If you don't receive a TXFE from the transceiver you have to resend the message somehow
                txamp = GPIO_PIN_RESET;
                // Open the PA before you send the packet
                HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
                xQueueReceive(outgoingTMQueue, &TM_PACKET, portMAX_DELAY);
                if (receivedEventsTransmit & TM_OBC) {
                    CAN::Application::getStoredMessage(&TM_PACKET, TX_BUFF, TM_PACKET.size, sizeof(TX_BUFF) / sizeof(TX_BUFF[0]));
                    LOG_DEBUG << "Received TM from OBC... preparing the transmission";
                }
                if (receivedEventsTransmit & TM_COMMS)
                    LOG_DEBUG << "Received TM from COMMS... preparing the transmission";
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
                                /// send the packet
                                //  counter++;
                                //  packetTestData.packet[0] = counter;
                                LOG_DEBUG << "[TX] TX PACKET SIZE: " << TM_PACKET.size;
                                // txamp = GPIO_PIN_RESET;
                                // HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
                                transceiver.transmitBasebandPacketsTx(RF09, TX_BUFF, TM_PACKET.size, error);
                                // LOG_INFO << "[TX] c: " << counter;
                                tx_counter++;
                                LOG_INFO << "[TX] TX counter: " << tx_counter;
                            }
                            xSemaphoreGive(transceiver_handler.resources_mtx);
                        }
                        break;
                    }
                    case TX_ONG: {
                        uint32_t receivedEventsTXFE;
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TXFE_TX, pdFALSE, pdTRUE, &receivedEventsTXFE, pdTICKS_TO_MS(1000)) == pdTRUE) {
                            if (receivedEventsTXFE & TXFE) {
                                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                                    if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                        ensureTxMode();
                                        /// Set the down-link frequency
                                        // transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
                                        // transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                                        // transceiver.chip_reset(error);
                                        // transceiver.setup(error);
                                        /// send the packet
                                        // counter++;
                                        // packetTestData.packet[0] = counter;
                                        // txamp = GPIO_PIN_RESET;
                                        // HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
                                        transceiver.transmitBasebandPacketsTx(RF09, TX_BUFF, TM_PACKET.size, error);
                                        // LOG_INFO << "[TX TXFE] c: " << counter;
                                        tx_counter++;
                                        LOG_DEBUG << "[TXFE] TX PACKET SIZE: " << TM_PACKET.size;
                                        LOG_INFO << "[TXFE] TX counter: " << tx_counter;
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
                                        // transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
                                        // transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
                                        // transceiver.chip_reset(error);
                                        // transceiver.setup(error);
                                        // send the packet
                                        // counter++;
                                        // packetTestData.packet[0] = counter;
                                        txamp = GPIO_PIN_RESET;
                                        HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
                                        transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                                        // LOG_INFO << "[TX RXFE] c: " << counter;
                                        tx_counter++;
                                        LOG_DEBUG << "[RXFE] TX PACKET SIZE: " << TM_PACKET.size;
                                        LOG_INFO << "[RXFE] TX counter: " << tx_counter;
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
        }
    }
}


