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
            vTaskDelay(pdMS_TO_TICKS(10));
            LOG_DEBUG << "[TX ENSURE] STATE: TRANSITION";
        break;
        case RF_RESET:
            LOG_DEBUG << "[TX ENSURE] STATE: RESET";
        break;
        case RF_INVALID:
            LOG_DEBUG << "[TX ENSURE] STATE: INVALID";
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(20));
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(10));
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

void RF_TXTask::transmitWithWait(uint8_t* tx_buf, uint16_t length, uint16_t wait_ms_for_txfe, Error& error) {
    ensureTxMode();
    transceiver.transmitBasebandPacketsTx(RF09, tx_buf, length, error);
    for (int i = 0; i < length; i++) {
        __NOP();
        // LOG_DEBUG << "[TX DATA] " << tx_buf[i];
    }

    if (xSemaphoreTake(transceiver_handler.txfeSemaphore_tx, pdMS_TO_TICKS(wait_ms_for_txfe)) == pdTRUE) {
        txfe_counter++;
        LOG_DEBUG << "[TX] TXFE: " << txfe_counter << " [TX] LENGTH: " << length - MAGIC_NUMBER;
        transceiver.tx_ongoing = false;
    }
    else {
        vTaskDelay(pdMS_TO_TICKS(1000));
        txfe_not_received++;
        // TODO : RESEND THE PACKET
        LOG_ERROR << "[TX READY] TXFE **NOT** RECEIVED: " << txfe_not_received;
        HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(20));
        HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(100));
        transceiver.set_state(RF09, RF_TRXOFF, error);
        transceiver.chip_reset(error);
        transceiver.tx_ongoing = false;
        /// TODO: RESEND
    }
}


[[noreturn]] void RF_TXTask::execute() {
    TXQueue = xQueueCreateStatic(TXQueueItemNum, TXItemSize, TXQueueStorageArea,
                                            &TXQueueBuffer);
    vQueueAddToRegistry(TXQueue, "RF TX queue");
    uint8_t state = 0;
    uint32_t receivedEventsTransmit = 0;

    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TRANSMIT, pdFALSE, pdTRUE, &receivedEventsTransmit, portMAX_DELAY) == pdTRUE) {
            while (uxQueueMessagesWaiting(TXQueue)) {
                /// TODO: If you don't receive a TXFE from the transceiver you have to resend the message somehow
                HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_RESET);
                xQueueReceive(TXQueue, &tx_handler, portMAX_DELAY);
                if (receivedEventsTransmit & TC_UART_TC_HANDLING_TASK ) {
                    for (int i = 0 ; i < tx_handler.data_length; i++) {
                        outgoing_TX_BUFF[i] = tx_handler.pointer_to_data[i];
                        LOG_INFO << "[TX] Data to send to the air from UART: " << outgoing_TX_BUFF[i];
                    }
                }
                if (receivedEventsTransmit & TM_OBC) {
                    // TODO: Do that inside of the TMHandlingTask
                    LOG_DEBUG << "[TX] New TM received with size: " << tx_handler.data_length;
                    for (int i = 0; i < tx_handler.data_length; i++) {
                        // LOG_DEBUG << "[TX] TM DATA: " << tx_handler.pointer_to_data[i];
                        outgoing_TX_BUFF[i] = tx_handler.pointer_to_data[i];
                    }
                }
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing;
                    switch (state) {
                        case READY: {
                                LOG_DEBUG << "[TX] READY";
                                transmitWithWait(outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, 250, error);
                                rf_rxtask->ensureRxMode();
                                HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
                                break;
                        }
                        case TX_ONG: {
                                LOG_DEBUG << "[TX] TX_ONG";
                                // if (xSemaphoreTake(transceiver_handler.txfeSemaphore_tx, pdMS_TO_TICKS(500))) {
                                //     if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                //         ensureTxMode();
                                //         transceiver.transmitBasebandPacketsTx(RF09, outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, error);
                                //         tx_counter++;
                                //         LOG_INFO << "[TXFE] TX counter: " << tx_counter;
                                //         rf_rxtask->ensureRxMode();
                                //     }
                                // }
                                // else {
                                //     LOG_ERROR << "[TX] TXFE NOT RECEIVED";
                                //     transceiver.set_state(RF09, RF_TRXOFF, error);
                                //     transceiver.chip_reset(error);
                                //     transceiver.tx_ongoing = false;
                                //     vTaskDelay(pdMS_TO_TICKS(100));
                                //     rf_rxtask->ensureRxMode();
                                //     // TODO: Send it again
                                // }
                            break;
                        }
                        case RX_ONG: {
                                LOG_DEBUG << "[TX] RX_ONG";
                                if (xSemaphoreTake(transceiver_handler.rxfeSemaphore_tx, pdMS_TO_TICKS(250))) {
                                    transmitWithWait(outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, 250, error);
                                    LOG_INFO << "[RXFE] TX counter: " << tx_counter;
                                }
                                else {
                                    LOG_ERROR << "[TX] RXFE NOT RECEIVED";
                                    transceiver.set_state(RF09, RF_TRXOFF, error);
                                    transceiver.chip_reset(error);
                                    transceiver.rx_ongoing = false;
                                    // TODO: Send it again
                                }
                                rf_rxtask->ensureRxMode();
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
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
            }
        }
    }
}

