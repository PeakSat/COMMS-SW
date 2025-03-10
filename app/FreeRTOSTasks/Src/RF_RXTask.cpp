#include "RF_RXTask.hpp"
#include "Logger.hpp"
#include <ECSS_Definitions.hpp>
#include <GNSS.hpp>
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
            vTaskDelay(pdMS_TO_TICKS(20));
            transceiver.set_state(RF09, RF_RX, error);
            break;
        case RF_TX:
            LOG_DEBUG << "[RX ENSURE] STATE: TX";
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(20));
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
            transceiver.set_state(RF09, RF_TXPREP, error);
            /// the delay here is essential
            vTaskDelay(pdMS_TO_TICKS(20));
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
            vTaskDelay(pdMS_TO_TICKS(20));
            HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(20));
            transceiver.set_state(RF09, RF_TRXOFF, error);
            vTaskDelay(pdMS_TO_TICKS(20));
            transceiver.set_state(RF09, RF_TXPREP, error);
            /// the delay here is essential
            vTaskDelay(pdMS_TO_TICKS(20));
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

bool RF_RXTask::verifyCRC(uint8_t* RX_BUFF, int32_t corrected_received_length) {
    if (corrected_received_length < CRC_LENGTH) {
        LOG_ERROR << "[RX] Packet too short to contain CRC!";
        return false;
    }
    uint32_t crc_received =
        (static_cast<uint32_t>(RX_BUFF[corrected_received_length - 4])) |
        (static_cast<uint32_t>(RX_BUFF[corrected_received_length - 3]) << 8) |
        (static_cast<uint32_t>(RX_BUFF[corrected_received_length - 2]) << 16) |
        (static_cast<uint32_t>(RX_BUFF[corrected_received_length - 1]) << 24);

    uint32_t crc_value = HAL_CRC_Calculate(&hcrc, reinterpret_cast<uint32_t*>(RX_BUFF), corrected_received_length - CRC_LENGTH);

    if (crc_value != crc_received) {
        //TODO ST[05]
        LOG_ERROR << "[RX] WRONG CRC RECEPTION";
        return false;
    }
    return true;
}

ParsedPacket RF_RXTask::parsePacket(const uint8_t* RX_BUFF) {
    ParsedPacket packet{};
    packet.packet_version_number = (RX_BUFF[0] >> 5) & 0x07;
    packet.packet_type = (RX_BUFF[0] >> 4) & 0x01;
    packet.secondary_header_flag = (RX_BUFF[0] >> 3) & 0x01;
    packet.application_process_ID = ((RX_BUFF[0] & 0x07) << 8) | RX_BUFF[1];
    //
    LOG_DEBUG << "Packet Version Number: " << packet.packet_version_number;
    LOG_DEBUG << "Packet Type: " << packet.packet_type;
    LOG_DEBUG << "Secondary Header Flag: " << packet.secondary_header_flag;
    LOG_DEBUG << "Application Process ID: " << packet.application_process_ID;
    return packet;
}


[[noreturn]] void RF_RXTask::execute() {
    vTaskDelay(pdMS_TO_TICKS(3000));
    LOG_INFO << "[RF RX TASK]";
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    /// ENABLE THE RX SWITCH
    HAL_GPIO_WritePin(EN_RX_UHF_GPIO_Port, EN_RX_UHF_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_RESET);
    /// Essential for the trx to be able to send and receive packets
    /// (If you have it HIGH from the CubeMX the trx will not be able to send)
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(20));
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
                break; // Exit loop if successful
            }

            attempt++;
            LOG_ERROR << "Retrying connection attempt " << attempt << "/" << MAX_RETRIES;
            vTaskDelay(pdMS_TO_TICKS(100)); // Small delay before retrying
        }
    }
    if (!success) {
        LOG_ERROR << "Failed to establish connection after " << MAX_RETRIES << " attempts.";
    }
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_SET);
    uint32_t rx_total_packets = 0, rx_total_drop_packets = 0, correct_packets = 0, error_rx_buf = 0;
    uint32_t receivedEvents = 0;
    memoryQueueItemHandler rf_rx_tx_queue_handler{};
    ensureRxMode();
    while (true) {
        if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RXFE_RX, pdFALSE, pdTRUE, &receivedEvents, portMAX_DELAY) == pdTRUE) {
            rx_total_packets++;
            if (xSemaphoreTake(transceiver_handler.resources_mtx, portMAX_DELAY) == pdTRUE) {
                HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
                auto result = transceiver.get_received_length(RF09, error);
                int32_t received_length = result.value();
                int32_t corrected_received_length = received_length - MAGIC_NUMBER;
                LOG_DEBUG << "[RX AGC] RECEIVED LENGTH: " << corrected_received_length;
                LOG_DEBUG << "[RX] TOTAL DROP: " << rx_total_drop_packets;
                LOG_DEBUG << "[RX] total packets c: " << rx_total_packets;
                LOG_DEBUG << "[RX] OKAY(CRC) PACKETS: " << correct_packets;
                LOG_DEBUG << "[RX] ERROR RX BUF: " << error_rx_buf;
                // TODO: SPACECRAFT ID FILTERING
                if (corrected_received_length >= MIN_TC_SIZE && corrected_received_length <= MAX_TC_SIZE) {
                    for (int i = 0; i < corrected_received_length; i++) {
                        RX_BUFF[i] = transceiver.spi_read_8((BBC0_FBRXS) + i, error);
                        if (error != NO_ERRORS) {
                            // TODO: ST[05]
                            LOG_ERROR << "ERROR";
                            error_rx_buf++;
                        }
                    }
                    if (verifyCRC(RX_BUFF, corrected_received_length)) {
                        correct_packets++;
                        ParsedPacket packet = parsePacket(RX_BUFF);
                        switch (packet.packet_type) {
                            case Message::PacketType::TC: {
                                LOG_DEBUG << "[RX] NEW TC RECEIVED";
                                rf_rx_tx_queue_handler.size = corrected_received_length - CRC_LENGTH;
                                auto status = storeItemInQueue(eMMC::memoryQueueMap[eMMC::rf_rx_tc], &rf_rx_tx_queue_handler, RX_BUFF, rf_rx_tx_queue_handler.size);
                                if (status.has_value()) {
                                    xQueueSendToBack(rf_rx_tcQueue, &rf_rx_tx_queue_handler, 0);
                                    tcHandlingTask->tc_rf_rx_var = true;
                                    xTaskNotifyIndexed(tcHandlingTask->taskHandle, NOTIFY_INDEX_INCOMING_TC, (1 << 19), eSetBits);
                                } else {
                                    // TODO: ST[05]
                                    LOG_ERROR << "[RX AGC] Failed to store MMC packet.";
                                }
                                break;
                            }
                            case Message::PacketType::TM: {
                                // TODO: SEND IT TO TM_HANDLING TASK
                                Message message = MessageParser::parse(RX_BUFF, corrected_received_length);
                                switch (message.packetType) {
                                    case 13: //ST[13]
                                        uint32_t headerSize = 15;
                                        uint8_t* dataPointer = RX_BUFF + headerSize; // ignore headers
                                        GNSSReceiver::parseGNSSData(dataPointer, corrected_received_length - headerSize);
                                        break;
                                }
                                LOG_DEBUG << "[RX] TM RECEPTION";
                                break;
                            }
                            default: {
                                // TODO: ST[05]
                                LOG_ERROR << "[RX] UNKNOWN PACKET TYPE: " << packet.packet_type;
                                break;
                            }
                        }
                    } else {
                        // TODO: ST[05]
                        LOG_ERROR << "[RX] WRONG CRC";
                        rx_total_drop_packets++;
                        ensureRxMode();
                    }
                } else {
                    // TODO: ST[05]
                    vTaskDelay(pdMS_TO_TICKS(100));
                    LOG_ERROR << "[RX] WRONG LENGTH";
                    rx_total_drop_packets++;
                    ensureRxMode();
                }
                ensureRxMode();
                xSemaphoreGive(transceiver_handler.resources_mtx);
            }
        }
    }
}
