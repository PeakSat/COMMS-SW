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


[[noreturn]] void RF_TXTask::execute() {
    TXQueue = xQueueCreateStatic(TXQueueItemNum, TXItemSize, TXQueueStorageArea,
                                            &TXQueueBuffer);
    vQueueAddToRegistry(TXQueue, "RF TX queue");
    uint8_t state = 0;
    uint32_t receivedEventsTransmit = 0;
    uint32_t tx_counter = 0;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TRANSMIT, pdFALSE, pdTRUE, &receivedEventsTransmit, portMAX_DELAY) == pdTRUE) {
            while (uxQueueMessagesWaiting(TXQueue)) {
                /// TODO: If you don't receive a TXFE from the transceiver you have to resend the message somehow
                xQueueReceive(TXQueue, &tx_handler, portMAX_DELAY);
                if (receivedEventsTransmit & TC_UART_TC_HANDLING_TASK) {
                    for (int i = 0 ; i < tx_handler.data_length; i++) {
                        outgoing_TX_BUFF[i] = tx_handler.pointer_to_data[i];
                        LOG_INFO << "[TX] Data to send to the air from UART: " << outgoing_TX_BUFF[i];
                    }
                }
                if (receivedEventsTransmit & TM_OBC) {
                    LOG_DEBUG << "[TX] New TM received with size: " << tx_handler.data_length;
                    for (int i = 0; i < tx_handler.data_length; i++) {
                        LOG_DEBUG << "[TX] TM DATA: " << tx_handler.pointer_to_data[i];
                        outgoing_TX_BUFF[i] = tx_handler.pointer_to_data[i];
                    }
                }
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing;
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
                // Open the TX Amplifier
                HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_RESET);
                switch (state) {
                    case READY: {
                        if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                            if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                ensureTxMode();
                                LOG_DEBUG << "[TX] TX PACKET SIZE: " << tx_handler.data_length;
                                transceiver.transmitBasebandPacketsTx(RF09, outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, error);
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
                                        transceiver.transmitBasebandPacketsTx(RF09, outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, error);
                                        tx_counter++;
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
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RXFE_TX, pdFALSE, pdTRUE, &receivedEventsRXFE, pdTICKS_TO_MS(1000)) == pdTRUE) {
                            if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                                if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                    ensureTxMode();
                                    transceiver.transmitBasebandPacketsTx(RF09, outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, error);
                                    tx_counter++;
                                    LOG_INFO << "[RXFE] TX counter: " << tx_counter;
                                }
                                xSemaphoreGive(transceiver_handler.resources_mtx);
                            }
                        }
                    }
                    break;
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

