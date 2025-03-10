#include "RF_TXTask.hpp"
#include "RF_RXTask.hpp"
#include "Logger.hpp"
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

void RF_TXTask::transmitWithWait(uint8_t* tx_buf, uint16_t length, uint16_t wait_ms_for_txfe, Error& error, bool enableCRC) {
    ensureTxMode();
    uint8_t local_tx_buf[2048];
    for (int i = 0; i < length; i++) {
        local_tx_buf[i] = tx_buf[i];
    }
    if (enableCRC) {
        uint32_t crc_value = HAL_CRC_Calculate(&hcrc, reinterpret_cast<uint32_t*>(local_tx_buf), length);
        LOG_DEBUG << "[TX]: CRC TRANSMIT: " << crc_value;
        local_tx_buf[length] = static_cast<uint8_t>(crc_value & 0xFF);              // 0x78
        local_tx_buf[length + 1] = static_cast<uint8_t>((crc_value >> 8)  & 0xFF);  // 0x56
        local_tx_buf[length + 2] = static_cast<uint8_t>((crc_value >> 16) & 0xFF);  // 0x34
        local_tx_buf[length + 3] = static_cast<uint8_t>((crc_value >> 24) & 0xFF);  // 0x12
        length = length + 4;
    }
    transceiver.transmitBasebandPacketsTx(RF09, local_tx_buf, length  + MAGIC_NUMBER, error);
    if (error != NO_ERRORS){
        //TODO ST[05]
        LOG_ERROR << "[TX TRANSMIT] ERROR: " << error;
    }
    if (xSemaphoreTake(transceiver_handler.txfeSemaphore_tx, pdMS_TO_TICKS(wait_ms_for_txfe)) == pdTRUE) {
        txfe_counter++;
        LOG_DEBUG << "[TX] TXFE: " << txfe_counter << " [TX] LENGTH: " << length - MAGIC_NUMBER;
        LOG_DEBUG << "[TX] TXFE NOT RECEIVED: " << txfe_not_received;
        LOG_DEBUG << "[TX] RXFE: " << rxfe_received << "[TX] RXFE NOT RECEIVED: " << rxfe_not_received;
        LOG_DEBUG << "[TX] TX_ONG COUNTER: " << tx_ong_counter;
        transceiver.tx_ongoing = false;
        HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
        txfe_not_received++;
        // TODO : RESEND THE PACKET
        HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(20));
        HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(100));
        transceiver.set_state(RF09, RF_TRXOFF, error);
        transceiver.chip_reset(error);
        transceiver.tx_ongoing = false;
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
                TX_PACKET_HANDLER tx_handler{};
                xQueueReceive(TXQueue, &tx_handler, portMAX_DELAY);
                if (receivedEventsTransmit & TC_UART_TC_HANDLING_TASK) {
                    for (int i = 0; i < tx_handler.data_length; i++) {
                        outgoing_TX_BUFF[i] = tx_handler.buf[i];
                        LOG_INFO << "[TX] Data to send to the air from UART: " << outgoing_TX_BUFF[i];
                    }
                }
                if (receivedEventsTransmit & TM_OBC) {
                    // TODO: Do that inside of the TMHandlingTask
                    LOG_DEBUG << "[TX] New TM received with size: " << tx_handler.data_length;
                    for (int i = 0; i < tx_handler.data_length; i++) {
                        outgoing_TX_BUFF[i] = tx_handler.buf[i];
                    }
                }
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing;
                    switch (state) {
                        case READY: {
                            LOG_DEBUG << "[TX] READY";
                            transmitWithWait(outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, 250, error, true);
                            rf_rxtask->ensureRxMode();
                            HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
                            break;
                        }
                        case TX_ONG: {
                            LOG_DEBUG << "[TX] TX_ONG";
                            tx_ong_counter++;
                            break;
                        }
                        case RX_ONG: {
                            LOG_DEBUG << "[TX] RX_ONG";
                            if (xSemaphoreTake(transceiver_handler.rxfeSemaphore_tx, pdMS_TO_TICKS(250))) {
                                rxfe_received++;
                                transmitWithWait(outgoing_TX_BUFF, tx_handler.data_length + MAGIC_NUMBER, 250, error, true);
                                transceiver.rx_ongoing = false;
                            } else {
                                rxfe_not_received++;
                                transceiver.set_state(RF09, RF_TRXOFF, error);
                                transceiver.chip_reset(error);
                                transceiver.rx_ongoing = false;
                                // TODO: Send it again
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
                    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
                    rf_rxtask->ensureRxMode();
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
            }
        }
    }
}
