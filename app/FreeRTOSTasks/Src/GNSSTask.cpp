#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"


static uint16_t size;

void GNSSTask::printing() {
    printing_counter++;
    if (printing_counter == printing_frequency) {
        printing_counter = 0;
        uint8_t* processingBuffer;
        processingBuffer = active_buffer_pointer;
        LOG_DEBUG << "Active buffer address: " << static_cast<uint8_t*>(processingBuffer);
        GNSSMessageString.assign(processingBuffer, processingBuffer + EXPECTED_GNSS_MESSAGE);
        LOG_DEBUG << GNSSMessageString.c_str();
        // Clear the previous data
        GNSSMessageString.clear();
    }
}

void GNSSTask::switchGNSSMode() {
    uint8_t interval;
    if (interval_mode_flag) {
        interval = 2;
        LOG_DEBUG << "FAST MODE";
    } else {
        interval = 15;
        LOG_DEBUG << "SLOW MODE";
    }
    // All NMEA strings are updated with the same interval even if you pass as an argument one string
    auto status = controlGNSSwithNotify(gnssReceiver.configureNMEAStringInterval("RMC", interval, GNSSDefinitions::Attributes::UpdateToSRAM));
    if (status.has_value())
        LOG_DEBUG << "GNSS INTERVAL CHANGED WITH SUCCESS";
    else
        LOG_ERROR << "GNSS INTERVAL COULD NOT BE CHANGED";

    interval_mode_flag = !interval_mode_flag;
}


etl::expected<void, ErrorFromGNSS> GNSSTask::controlGNSSwithNotify(GNSSMessage gnssMessageToSend) {
    uint8_t* processingBuffer;
    processingBuffer = active_buffer_pointer;

    const uint8_t maxRetries = 3;

    // Try up to maxRetries to receive an ACK/NACK
    for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
        if (HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000) != HAL_OK) {
            LOG_ERROR << "Transmission failed";
            return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::TransmissionFailed); // Return TransmissionFailed error
        }

        // Wait for ACK or NACK with a timeout
        uint32_t receivedEvents = 0;

        if (xTaskNotifyWait(GNSS_RESPONSE, GNSS_RESPONSE, &receivedEvents, pdMS_TO_TICKS(1000)) == pdTRUE) {
            // Check if we received a response
            if (receivedEvents & GNSS_RESPONSE) {
                for (uint16_t i = 0; i < size; i++) {
                    if (processingBuffer[i] == 131) {
                        // ACK received
                        LOG_DEBUG << "ACK received!";
                        return {}; // Success case (void), indicating ACK received
                    }
                    if (processingBuffer[i] == 132) {
                        // NACK received
                        LOG_ERROR << "NACK received!";
                        return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::NACKReceived); // Return NACKReceived error
                    }
                }
            }
        }
    }
    // If we exit the loop without success, return a Timeout error
    LOG_ERROR << "Failed to receive ACK/NACK after maximum retries.";
    return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::Timeout);
}


void GNSSTask::execute() {

    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_RSTN_GPIO_Port, GNSS_RSTN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);

    active_buffer_pointer = const_cast<uint8_t*>(first_buffer);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, const_cast<uint8_t*>(first_buffer), 512);

    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
    //  disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);

    vTaskDelay(500);

    uint32_t receivedEvents = 0;
    uint8_t counter = 0;

    while (true) {
        xTaskNotifyWait(GNSS_MESSAGE_READY, GNSS_MESSAGE_READY, &receivedEvents, portMAX_DELAY);
        if (receivedEvents & GNSS_MESSAGE_READY) {
            printing();
            counter++;
        }
        if (counter == 4) {
            // Switch mode every 4 messages
            if (counter == 4) {
                counter = 0;
                switchGNSSMode();
            }
        }
    }
}
