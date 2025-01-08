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
    vTaskDelay(5000);
    LOG_INFO << "RF TX TASK";
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    LOG_INFO << "5V CHANNEL ON";
    /// Essential for the trx to be able to send and receive packets
    /// (If you have it HIGH from the CubeMX the trx will not be able to send)
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_RESET);
    vTaskDelay(20);
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
    /// Check transceiver connection
    auto status = transceiver.check_transceiver_connection(error);
    if (status.has_value()) {
        LOG_INFO << "AT86RF215 CONNECTION OK";
    } else
        /// TODO: Error handling
        LOG_ERROR << "AT86RF215 ##ERROR## WITH CODE: " << status.error();
    /// Set the down-link frequency
    transceiver.freqSynthesizerConfig.setFrequency_FineResolution_CMN_1(FrequencyUHFTX);
    transceiver.configure_pll(AT86RF215::RF09, transceiver.freqSynthesizerConfig.channelCenterFrequency09, transceiver.freqSynthesizerConfig.channelNumber09, transceiver.freqSynthesizerConfig.channelMode09, transceiver.freqSynthesizerConfig.loopBandwidth09, transceiver.freqSynthesizerConfig.channelSpacing09, error);
    transceiver.chip_reset(error);
    transceiver.setup(error);
    /// TX AMP
    GPIO_PinState txamp = GPIO_PIN_SET;
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, txamp);
    if (txamp)
        LOG_DEBUG << "TX AMP DISABLED";
    else
        LOG_DEBUG << "TX AMP ENABLED";

    uint32_t receivedEvents = 0;
    while (1) {
        vTaskDelay(100);
        /// Read the state
        uint8_t read_reg = transceiver.spi_read_8(RF09_STATE, error);
        LOG_DEBUG << "Transceiver State = " << read_reg;
        PacketData packetTestData = createRandomPacketData(MaxPacketLength);
        /// Writes to the TX buffer
        transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &receivedEvents, pdMS_TO_TICKS(100))) {
            if (receivedEvents & TXFE) {
                LOG_DEBUG << "PACKET IS SENT WITH SUCCESS, LENGTH: " << packetTestData.length;
            }
            /// Handle TRXRDY flag
            if (receivedEvents & TRXRDY)
                LOG_DEBUG << "TRANSCEIVER IS READY.";
            /// Handle TRXRDY flag
            if (receivedEvents & TRXERR)
                LOG_ERROR << "TRANSCEIVER ERROR.";
            /// Handle IFSERR flag
            if (receivedEvents & IFSERR)
                LOG_ERROR << "SYNCHRONIZATION ERROR.";
            if (receivedEvents & WAKEUP)
                LOG_DEBUG << "WAKE UP";
            if (receivedEvents & FBLI)
                LOG_ERROR << "FRAME BUFFER LEVEL INDICATION";
        }
    }
}