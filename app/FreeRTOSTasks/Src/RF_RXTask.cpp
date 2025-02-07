#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <RF_TXTask.hpp>
#include <TCHandlingTask.hpp>
#include <eMMC.hpp>
#define MAGIC_NUMBER 4
using namespace AT86RF215;

void RF_RXTask::ensureRxMode() {
    switch (State trx_state = transceiver.get_state(RF09, error)) {
        case RF_NOP:
            LOG_DEBUG << "[RX ENSURE] STATE: NOP";
            break;
        case RF_SLEEP:
            LOG_DEBUG << "[RX ENSURE] STATE: SLEEP";
            break;
        case RF_TRXOFF:
            LOG_DEBUG << "[RX ENSURE] STATE: TRXOFF";
            transceiver.set_state(RF09, RF_TXPREP, error);
            /// the delay here is essential
            vTaskDelay(20);
            transceiver.set_state(RF09, RF_RX, error);
            break;
        case RF_TX:
            LOG_DEBUG << "[RX ENSURE] STATE: TX";
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
            vTaskDelay(20);
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
            transceiver.set_state(RF09, RF_TXPREP, error);
            /// the delay here is essential
            vTaskDelay(20);
            transceiver.set_state(RF09, RF_RX, error);
            // transceiver.print_state(RF09, error);
            break;
        case RF_RX:
            // LOG_DEBUG << "[RX ENSURE] STATE: RX";
            break;
        case RF_TRANSITION:
            LOG_DEBUG << "[RX ENSURE] STATE: TRANSITION";
            break;
        case RF_RESET:
            LOG_DEBUG << "[RX ENSURE] STATE: RESET";
            break;
        case RF_INVALID:
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
            vTaskDelay(20);
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
            vTaskDelay(10);
            transceiver.set_state(RF09, RF_TRXOFF, error);
            vTaskDelay(10);
            transceiver.set_state(RF09, RF_TXPREP, error);
            /// the delay here is essential
            vTaskDelay(20);
            transceiver.set_state(RF09, RF_RX, error);
            LOG_DEBUG << "[RX ENSURE] STATE: INVALID";
            transceiver.print_state(RF09, error);
            break;
        case RF_TXPREP:
            LOG_DEBUG << "[RX ENSURE] STATE: TXPREP";
            transceiver.set_state(RF09, RF_RX, error);
            break;
        default:
            LOG_ERROR << "[RX ENSURE] STATE: UNDEFINED";
            break;
    }
}

[[noreturn]] void RF_RXTask::execute() {
    vTaskDelay(5000);
    LOG_INFO << "[RF RX TASK]";
    incomingTCQueue = xQueueCreateStatic(TCQueueSize, sizeof(CAN::StoredPacket), incomingTCQueueStorageArea,
                                            &incomingTCQueueBuffer);
    vQueueAddToRegistry(incomingTCQueue, "TC queue");
    /// Set the Up-link frequency
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    /// ENABLE THE RX SWITCH
    HAL_GPIO_WritePin(EN_RX_UHF_GPIO_Port, EN_RX_UHF_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_RESET);
    /// Essential for the trx to be able to send and receive packets
    /// (If you have it HIGH from the CubeMX the trx will not be able to send)
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
    vTaskDelay(20);
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
    transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFRX);
    const int MAX_RETRIES = 3;
    int attempt = 0;
    bool success = false;
    while (attempt < MAX_RETRIES) {
        if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
            auto status = transceiver.check_transceiver_connection(error);
            if (status.has_value()) {
                success = true;  // Connection successful
                LOG_INFO << "[SPI CONNECTION ESTABLISHED]";
            } else {
                LOG_ERROR << "CONNECTION ##ERROR## WITH CODE: " << status.error();
            }
            transceiver.set_state(RF09, RF_TRXOFF, error);
            transceiver.configure_pll(RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09,
                                      transceiver.freqSynthesizerConfig.channelNumber09,
                                      transceiver.freqSynthesizerConfig.channelMode09,
                                      transceiver.freqSynthesizerConfig.loopBandwidth09,
                                      transceiver.freqSynthesizerConfig.channelSpacing09, error);
            transceiver.chip_reset(error);

            xSemaphoreGive(transceiver_handler.resources_mtx);

            if (success) {
                break;  // Exit loop if successful
            }

            attempt++;
            LOG_ERROR << "Retrying connection attempt " << attempt << "/" << MAX_RETRIES;
            vTaskDelay(pdMS_TO_TICKS(100));  // Small delay before retrying
        }
    }
    if (!success) {
        LOG_ERROR << "Failed to establish connection after " << MAX_RETRIES << " attempts.";
    }
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_SET);
    uint16_t received_length = 0;
    uint8_t current_counter = 0;
    uint32_t drop_counter = 0;
    uint32_t receivedEvents;
    ensureRxMode();
    uint32_t rx_total_packets = 0;
    uint32_t rx_total_drop_packets = 0;
    GPIO_PinState txamp;
    State trx_state;
    CAN::StoredPacket PacketToBeStored;
    uint32_t eMMCPacketTailPointer = 0;
    uint8_t counter_tx_ong;
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_AGC, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(transceiver_handler.RX_REFRESH_PERIOD_MS)) == pdTRUE) {
                if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                    auto result = transceiver.get_received_length(RF09, error);
                    received_length = result.value();
                    int8_t rssi = transceiver.get_rssi(RF09, error);
                    if (rssi != 127)
                        LOG_DEBUG << "[RX AGC] RSSI [dBm]: " << rssi ;
                    transceiver.print_error(error);
                    if (received_length) {
                        LOG_DEBUG << "[RX AGC] LENGTH: " << received_length - MAGIC_NUMBER;
                        rx_total_packets++;
                        LOG_DEBUG << "[RX] total packets c: " << rx_total_packets;
                        drop_counter = 0;
                        for (int i = 0; i < received_length - MAGIC_NUMBER; i++) {
                            RX_BUFF[i] = transceiver.spi_read_8((BBC0_FBRXS) + i, error);
                        }
                        auto status = storeItem(eMMC::memoryMap[eMMC::RECEIVED_TC], RX_BUFF, 512, eMMCPacketTailPointer, 1);
                        if (status.has_value()) {
                            PacketToBeStored.pointerToeMMCItemData = eMMCPacketTailPointer;
                            PacketToBeStored.size = received_length - MAGIC_NUMBER;
                            eMMCPacketTailPointer += 1;
                            xQueueSendToBack(incomingTCQueue, &PacketToBeStored, 0);
                            xTaskNotifyIndexed(tcHandlingTask->taskHandle, NOTIFY_INDEX_RECEIVED_TC, 0, eNoAction);
                        }
                    }
                    else {
                        drop_counter++;
                        rx_total_drop_packets++;
                        LOG_DEBUG << "[RX DROP] c: " << drop_counter;
                        LOG_DEBUG << "[RX DROP] total packets c: " << rx_total_drop_packets;
                    }
                    xSemaphoreGive(transceiver_handler.resources_mtx);
                }
        }
        else {
            if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                switch (uint8_t rf_state = (transceiver.rx_ongoing << 1) | transceiver.tx_ongoing) {
                    case READY: {
                        trx_state = transceiver.get_state(RF09, error);
                        if (trx_state != RF_RX) {
                            // txamp = GPIO_PIN_SET;
                            // HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
                            ensureRxMode();
                        }
                        break;
                    }
                    case TX_ONG: {
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TXFE_RX, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(1000)) == pdTRUE) {
                            if (receivedEvents & TXFE) {
                                ensureRxMode();
                                LOG_INFO << "[RX] TXFE";
                            }
                        }
                        else {
                            LOG_ERROR << "[RX] TXFE NOT RECEIVED";
                            // DEADLOCK HANDLING
                            transceiver.print_error(error);
                            transceiver.print_state(RF09, error);
                            transceiver.set_state(RF09, RF_TRXOFF, error);
                            transceiver.chip_reset(error);
                            transceiver.print_error(error);
                            transceiver.tx_ongoing = false;
                            ensureRxMode();
                        }
                        // txamp = GPIO_PIN_SET;
                        // HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
                        break;
                    }
                    case RX_ONG: {
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RXFE_RX, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(1000))) {
                            if (receivedEvents &  RXFE_RX) {
                                trx_state = transceiver.get_state(RF09, error);
                                if (trx_state != RF_RX) {
                                    ensureRxMode();
                                }
                            }
                        }
                        trx_state = transceiver.get_state(RF09, error);
                        if (trx_state != RF_RX) {
                            ensureRxMode();
                        }
                        LOG_DEBUG << "[RX] RX_ONG";
                        break;
                    }
                    case RX_TX_ONG: {
                        LOG_DEBUG << "[RX] RXONG AND TXONG";
                        break;
                    }
                    default: {
                        LOG_ERROR << "[RX] UNEXPECTED CASE";
                        break;
                    }
                }
            }
            if (transceiver.TransceiverError_flag) {
                transceiver.TransceiverError_flag = false;
                LOG_ERROR << "[RX] Transceiver Error";
            }
            if (transceiver.FrameBufferLevelIndication_flag) {
                transceiver.FrameBufferLevelIndication_flag = false;
                LOG_ERROR << "[RX] FrameBuffer Level Indication";
            }
            if (transceiver.IFSynchronization_flag) {
                transceiver.IFSynchronization_flag = false;
                LOG_ERROR << "[RX] IF Synchronization";
            }
            if (transceiver.Voltage_Drop) {
                transceiver.Voltage_Drop = false;
                LOG_ERROR << "[RX] Voltage Drop";
            }
            if (transceiver.TransmitterFrameEnd_flag) {
                transceiver.TransmitterFrameEnd_flag = false;
                LOG_INFO << "[RX] Transceiver FRAME END";
            }
            xSemaphoreGive(transceiver_handler.resources_mtx);
        }
    }
}
