#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <ECSS_Definitions.hpp>
#include <Message.hpp>
#include <MessageParser.hpp>
#include <RF_TXTask.hpp>
#include <TCHandlingTask.hpp>
#include <TMHandlingTask.hpp>
#include <eMMC.hpp>
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
    vTaskDelay(4000);
    LOG_INFO << "[RF RX TASK]";
    incomingTCQueue = xQueueCreateStatic(TCQueueSize, sizeof(CAN::StoredPacket), incomingTCQueueStorageArea,
                                            &incomingTCQueueBuffer);
    vQueueAddToRegistry(incomingTCQueue, "TC queue");
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
    constexpr int MAX_RETRIES = 3;
    int attempt = 0;
    bool success = false;
    while (attempt < MAX_RETRIES) {
        if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
            auto status = transceiver.check_transceiver_connection(error);
            if (status.has_value()) {
                success = true;
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
    uint32_t drop_counter = 0;
    uint32_t rx_total_packets = 0;
    uint32_t rx_total_drop_packets = 0;
    uint32_t receivedEvents = 0;
    uint32_t eMMCPacketTailPointer = 0;
    State trx_state;
    CAN::StoredPacket PacketToBeStored;
    memoryQueueItemHandler rf_rx_tx_queue_handler{};
    ensureRxMode();

    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_AGC, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(transceiver_handler.RX_REFRESH_PERIOD_MS)) == pdTRUE) {
            if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                auto result = transceiver.get_received_length(RF09, error);
                received_length = result.value();
                int16_t corrected_received_length = received_length - MAGIC_NUMBER;
                int8_t rssi = transceiver.get_rssi(RF09, error);
                uint8_t RX_BUFF[1024]{};
                LOG_DEBUG << "[RX AGC] LENGTH: " << corrected_received_length;
                if (rssi != 127)
                    LOG_DEBUG << "[RX AGC] RSSI [dBm]: " << rssi ;
                if (corrected_received_length > 0 && corrected_received_length <= 128) {
                    LOG_DEBUG << "[RX AGC] LENGTH: " << corrected_received_length;
                    rx_total_packets++;
                    LOG_DEBUG << "[RX] total packets c: " << rx_total_packets;
                    drop_counter = 0;
                    for (int i = 0; i < corrected_received_length; i++) {
                        RX_BUFF[i] = transceiver.spi_read_8((BBC0_FBRXS) + i, error);
                        if (error != NO_ERRORS)
                            LOG_ERROR << "ERROR" ;
                    }
                    /// TODO: parse the packet because it could be a TM if we are on the COMMS-GS or TC if we are on the COMMS-GS side
                    if (RX_BUFF[1] == Message::PacketType::TM) {
                        LOG_DEBUG << "[RX AGC] NEW TM FROM OBC";
                    }
                    else if (RX_BUFF[1] == Message::PacketType::TC) {
                        LOG_DEBUG << "[RX AGC] NEW TC FROM COMMS-GS";
                        rf_rx_tx_queue_handler.size = corrected_received_length;
                        auto status = eMMC::storeItemInQueue(eMMC::memoryQueueMap[eMMC::rf_rx_tc], &rf_rx_tx_queue_handler, RX_BUFF, rf_rx_tx_queue_handler.size);
                        if (status.has_value()) {
                            if (rf_rx_tcQueue != nullptr) {
                                xQueueSendToBack(rf_rx_tcQueue, &rf_rx_tx_queue_handler, 0);
                                if (tcHandlingTask->taskHandle != nullptr) {
                                    tcHandlingTask->tc_rf_rx_var = true;
                                    xTaskNotifyIndexed(tcHandlingTask->taskHandle, NOTIFY_INDEX_INCOMING_TC, (1 << 19), eSetBits);
                                }
                                else {
                                    LOG_ERROR << "[RX] TC_HANDLING not started yet";
                                }
                            }
                        }
                        else {
                            LOG_ERROR << "[RX AGC] Failed to store MMC packet.";
                        }
                    }
                    else {
                        LOG_DEBUG << "[RX AGC] Neither TC nor TM";
                    }
                    /// TODO: if the packet is TM print it with the format: New TM [3,25] ... call the TM_HandlingTask
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
                            ensureRxMode();
                        }
                        break;
                    }
                    case TX_ONG: {
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_TXFE_RX, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(1000)) == pdTRUE) {
                            if (receivedEvents & TXFE) {
                                ensureRxMode();
                                LOG_INFO << "[RX] TXFE RECEIVED";
                            }
                        }
                        else {
                            LOG_ERROR << "[RX] TXFE NOT RECEIVED";
                            transceiver.print_state(RF09, error);
                            transceiver.set_state(RF09, RF_TRXOFF, error);
                            transceiver.chip_reset(error);
                            transceiver.tx_ongoing = false;
                            ensureRxMode();
                        }
                        HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
                        break;
                    }
                    case RX_ONG: {
                        LOG_DEBUG << "[RX] RX_ONG";
                        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RXFE_RX, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(1000))) {
                            if (receivedEvents &  RXFE_RX) {
                                LOG_INFO << "[RX] RXFE RECEIVED";
                                trx_state = transceiver.get_state(RF09, error);
                                if (trx_state != RF_RX) {
                                    ensureRxMode();
                                }
                            }
                        }
                        else {
                            transceiver.set_state(RF09, RF_TRXOFF, error);
                            transceiver.chip_reset(error);
                            transceiver.rx_ongoing = false;
                            ensureRxMode();
                            LOG_ERROR << "[RX] RXFE NOT RECEIVED";
                        }
                        break;
                    }
                    case RX_TX_ONG: {
                        LOG_ERROR << "[RX] RXONG AND TXONG";
                        break;
                    }
                    default: {
                        LOG_ERROR << "[RX] UNEXPECTED CASE";
                        break;
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
                    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
                }
                xSemaphoreGive(transceiver_handler.resources_mtx);
            }
        }
    }
}
