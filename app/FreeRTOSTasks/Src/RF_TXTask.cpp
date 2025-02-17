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
    TXQueue = xQueueCreateStatic(outgoingTXQueueSize, sizeof(CAN::StoredPacket), outgoingTXQueueStorageArea,
                                            &outgoingTXQueueBuffer);
    vQueueAddToRegistry(TXQueue, "TM outgoing queue");
    vTaskDelay(8000);
    uint8_t state = 0;
    uint8_t counter = 0;
    uint32_t receivedEventsTransmit;
    uint32_t tx_counter = 0;
    CAN::StoredPacket TX_PACKET;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TRANSMIT, pdFALSE, pdTRUE, &receivedEventsTransmit, pdTICKS_TO_MS(transceiver_handler.BEACON_PERIOD_MS + 1000)) == pdTRUE) {
            while (uxQueueMessagesWaiting(TXQueue)) {
                HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_RESET);
                /// TODO: If you don't receive a TXFE from the transceiver you have to resend the message somehow
                xQueueReceive(TXQueue, &TX_PACKET, portMAX_DELAY);
                if (receivedEventsTransmit & TC_UART_TC_HANDLING_TASK) {
                    auto status = getItem(eMMC::memoryMap[eMMC::UART_TC], TX_BUFF, 1024, TX_PACKET.pointerToeMMCItemData, 2);
                    if (status.has_value()) {
                        LOG_DEBUG << "[TX] Received TC... preparing its transmission to the air";
                    }
                    else
                        LOG_ERROR << "[TX] ERROR: memory";
                }
                if (receivedEventsTransmit & TM_OBC) {
                    CAN::Application::getStoredMessage(&TX_PACKET, TX_BUFF, TX_PACKET.size, sizeof(TX_BUFF) / sizeof(TX_BUFF[0]));
                    LOG_DEBUG << "Received TM from OBC... preparing the transmission";
                }
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing;
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
                for (uint32_t i = 0; i < TX_PACKET.size; i++) {
                    LOG_INFO << "TX PACKET to be sent: " << TX_BUFF[i];
                }
                switch (state) {
                    case READY: {
                        if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                            if (!transceiver.rx_ongoing && !transceiver.tx_ongoing) {
                                ensureTxMode();
                                LOG_DEBUG << "[TX] TX PACKET SIZE: " << TX_PACKET.size;
                                transceiver.transmitBasebandPacketsTx(RF09, TX_BUFF, TX_PACKET.size + 4, error);
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
                                        transceiver.transmitBasebandPacketsTx(RF09, TX_BUFF, TX_PACKET.size + 4, error);
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
                                    transceiver.transmitBasebandPacketsTx(RF09, TX_BUFF, TX_PACKET.size + 4, error);
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

