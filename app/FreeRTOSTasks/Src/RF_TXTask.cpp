#include "RF_TXTask.hpp"
#include "Logger.hpp"

PacketData RF_TXTask::createRandomPacketData(uint16_t length) {
    PacketData data{};
    for (int i = 0; i < length; i++)
        data.packet[i] = i % 100;
    data.length = length;
    return data;
}

void RF_TXTask::execute() {
    uint32_t receivedEvents = 0;
    if (xTaskNotifyWait(START_TX_TASK, START_TX_TASK, &receivedEvents, 10000) == pdTRUE) {
        if (receivedEvents & START_TX_TASK) {
            LOG_INFO << "Starting RF TX Task"; // Handle the event
        }
    } else {
        // In case the notification was not received, you can log or handle it
        LOG_ERROR << "RF RX Task notification error.";
    }
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
        LOG_DEBUG << "TX AMP DISABLED";
    else
        LOG_DEBUG << "TX AMP ENABLED";

    while (1) {
        vTaskDelay(1000);
        /// Read the state
        if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
            uint8_t read_reg = transceiver.spi_read_8(RF09_STATE, error);
            LOG_DEBUG << "Transceiver State = " << read_reg;
            PacketData packetTestData = createRandomPacketData(MaxPacketLength);
            /// Writes to the TX buffer
            transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
            if (xTaskNotifyWait(0, 0xFFFFFFFF, &receivedEvents, pdMS_TO_TICKS(100))) {
                if (receivedEvents & TXFE) {
                    /// Give the mutex
                    LOG_DEBUG << "PACKET IS SENT WITH SUCCESS, LENGTH: " << packetTestData.length;
                }
                /// Handle TRXRDY flag
                if (receivedEvents & TRXRDY)
                    LOG_DEBUG << "TRANSCEIVER IS READY.";
                /// Handle TRXRDY flag
                if (receivedEvents & TRXERR)
                    LOG_ERROR << "PLL UNLOCKED.";
                /// Handle IFSERR flag
                if (receivedEvents & IFSERR)
                    LOG_ERROR << "SYNCHRONIZATION ERROR.";
            }
            xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
        }
    }
}