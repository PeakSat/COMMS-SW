#include "RF_TXTask.hpp"
#include "Logger.hpp"

PacketData RF_TXTask::createRandomPacketData(uint8_t length) {
    PacketData data{};
    for (uint8_t i = 0; i < length; i++)
        data.packet[i] = i;
    data.length = length;
    return data;
}


void RF_TXTask::execute() {
    uint32_t receivedEvents = 0;
    if (xTaskNotifyWait(START_TX_TASK, START_TX_TASK, &receivedEvents, portMAX_DELAY) == pdTRUE) {
        if (receivedEvents & START_TX_TASK) {
            LOG_INFO << "Starting RF TX Task NOMINALLY"; // Handle the event
        }
    } else {
        // In case the notification was not received, you can log or handle it
        LOG_ERROR << "RF RX Task notification error.";
    }
    LOG_INFO << "Starting RF TX Task"; // Handle the event
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
        LOG_DEBUG << "##########TX AMP DISABLED##########";
    else
        LOG_DEBUG << "##########TX AMP ENABLED##########";
    int i = 1;
    uint8_t counter = 0;
    PacketData packetTestData = createRandomPacketData(MaxPacketLength);
    while (1) {
        /// without delay there is an issue on the receiver. The TX stops the transmission after a while and the receiver loses packets.
        /// But why ? There is also the delay of the TXFE interrupt which is around [1 / (50000 / (packetLenghtInbits)) s]. It should work just fine...
        /// Maybe there is an error either on the tx side or the rx side, which is not printed.
        /// Typically 200ms delay works.
        /// We had the same issue on the campaign.
        if (xTaskNotifyWait(TRANSMIT, TRANSMIT, &receivedEvents, portMAX_DELAY) == pdTRUE) {
            if (receivedEvents & TRANSMIT) {
                if (xSemaphoreTake(TransceiverHandler::transceiver_semaphore, portMAX_DELAY) == pdTRUE) {
                    if (counter == 255)
                        counter = 0;
                    // Prepare and send the packet
                    packetTestData.packet[0] = counter++;
                    transceiver.transmitBasebandPacketsTx(RF09, packetTestData.packet.data(), packetTestData.length, error);
                    transceiver.print_error(error);
                    LOG_INFO << "[TX TASK] Transmitted packet: " << counter;
                    transceiver.print_state(RF09, error);

                    xSemaphoreGive(TransceiverHandler::transceiver_semaphore);
                }
            }
        }
    }
}