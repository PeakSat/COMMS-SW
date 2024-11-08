#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"


extern bool ack_flag;
extern bool nack_flag;


static uint16_t size;

void GNSSTask::printing() {
    printing_counter++;
    if (printing_counter == printing_frequency) {
        printing_counter = 0;
        for (uint16_t i = 0; i < 460; i++) {
            GNSSMessageString.push_back(incomingMessage[i]);
        }
        LOG_DEBUG << GNSSMessageString.c_str();
        // Clear the previous data
        GNSSMessageString.clear();
    }
}


void GNSSTask::initializeNMEAStrings(etl::vector<etl::string<3>, 10>& nmeaStrings) {
    // Clear any existing data if necessary
    nmeaStrings.clear();
    // Push back the strings into the vector
    nmeaStrings.push_back("GGA");
    nmeaStrings.push_back("ZDA");
    nmeaStrings.push_back("GSA");
    nmeaStrings.push_back("GLL");
    nmeaStrings.push_back("VTG");
    nmeaStrings.push_back("RMC");
}

etl::expected<void, ErrorFromGNSS> GNSSTask::changeIntervalofNMEAStrings(etl::vector<etl::string<3>, 10>& nmeaStrings, uint8_t interval, Attributes attributes) {
    interval = nmea_string_interval;
    uint8_t string_counter = 0;
    for (auto& str: nmeaStrings) {
        string_counter++;
        auto status = controlGNSSwithNotify(gnssReceiver.configureNMEAStringInterval(str, interval, attributes));
        // Check if the configuration succeeded
        if (!status) {
            // If an error occurred, log it and return immediately with the error
            LOG_ERROR << "Failed to set interval for NMEA string: " << string_counter << " with error: " << static_cast<int>(status.error());
            return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::MultipleCommandsFail);
        }
    }
    return {};
}

void GNSSTask::controlGNSS(GNSSMessage gnssMessageToSend) {

    for (uint8_t byte: gnssMessageToSend.messageBody)
        LOG_DEBUG << byte;

    vTaskDelay(50);
    HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000);
    // Now wait for ACK or NACK response, with a timeout for safety
    uint32_t timeout = 1000;            // Timeout duration in ms (adjust as necessary)
    uint32_t startTime = HAL_GetTick(); // Get the current tick time

    while (HAL_GetTick() - startTime < timeout) {
        vTaskDelay(10);
        // Check if data is available in UART
        if (ack_flag) {
            // ACK received
            ack_flag = false;
            LOG_DEBUG << "ACK received!";
            return;
        } else if (nack_flag) {
            // NACK received
            nack_flag = false;
            LOG_ERROR << "NACK received!";
            return;
        }
    }
    LOG_DEBUG << "error timeout";
}

etl::expected<void, ErrorFromGNSS> GNSSTask::controlGNSSwithNotify(GNSSMessage gnssMessageToSend) {
    const uint8_t maxRetries = 3; // Maximum number of retries
    // Attempt to transmit the message
    if (HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000) != HAL_OK) {
        LOG_ERROR << "Transmission failed";
        return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::TransmissionFailed); // Return TransmissionFailed error
    }
    // Try up to maxRetries to receive an ACK/NACK
    for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
        // Wait for ACK or NACK with a timeout
        uint32_t receivedEvents = 0;
        if (xTaskNotifyWait(GNSS_RESPONSE, GNSS_RESPONSE, &receivedEvents, pdMS_TO_TICKS(1000)) == pdTRUE) {
            // Check if we received a response
            if (receivedEvents & GNSS_RESPONSE) {
                for (uint16_t i = 0; i < size; i++) {
                    if (gnssTask->incomingMessage[i] == 131) {
                        // ACK received
                        LOG_DEBUG << "ACK received!";
                        return {}; // Success case (void), indicating ACK received
                    }
                    if (gnssTask->incomingMessage[i] == 132) {
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

    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, const_cast<uint8_t*>(incomingMessage), 512);

    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
    //  disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);

    vTaskDelay(500);

    etl::vector<etl::string<3>, 10> nmeaStrings;
    initializeNMEAStrings(nmeaStrings);


    changeIntervalofNMEAStrings(nmeaStrings, 10, GNSSDefinitions::Attributes::UpdateToSRAM);
    uint32_t receivedEvents = 0;
    while (true) {
        xTaskNotifyWait(GNSS_MESSAGE_READY, GNSS_MESSAGE_READY, &receivedEvents, portMAX_DELAY);
        if (receivedEvents & GNSS_MESSAGE_READY)
            printing();
    }
}
