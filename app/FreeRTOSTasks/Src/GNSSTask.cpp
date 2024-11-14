#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"

void GNSSTask::printing(uint8_t* buf) {
    printing_counter++;
    if (printing_counter == printing_frequency) {
        printing_counter = 0;
        GNSSMessageString.assign(buf, buf + GNSSPayloadSize);
        LOG_DEBUG << GNSSMessageString.c_str();
        // Clear the previous data
        GNSSMessageString.clear();
        memset(buf, 0, 1024);
    }
}

void GNSSTask::switchGNSSMode() {
    etl::vector<uint8_t, 12> interval_vec;
    uint8_t seconds;
    if (interval_mode_flag) {
        LOG_DEBUG << "FAST MODE";
        interval_vec.resize(12, 0);
        // RMC update every 1 second
        seconds = 1;
        interval_vec[4] = seconds;
    } else {
        LOG_DEBUG << "SLOW MODE";
        interval_vec.resize(12, 0);
        // RMC update every 6 seconds
        seconds = 6;
        interval_vec[4] = seconds;
    }
    auto status = controlGNSSwithNotify(GNSSReceiver::configureExtendedNMEAMessageInterval(interval_vec, Attributes::UpdateToSRAM));
    if (status.has_value())
        LOG_DEBUG << "GNSS INTERVAL CHANGED WITH SUCCESS";
    else
        LOG_ERROR << "GNSS INTERVAL COULD NOT BE CHANGED";

    interval_mode_flag = !interval_mode_flag;
}

void GNSSTask::startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size) {
    // start the DMA
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, buf, size);
    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
    //  disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);
}


etl::expected<void, ErrorFromGNSS> GNSSTask::controlGNSSwithNotify(GNSSMessage gnssMessageToSend) {
    control = 1;
    const uint8_t maxRetries = 3;
    uint8_t* rx_buf_from_queue = nullptr;
    // Try up to maxRetries to receive an ACK/NACK
    for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
        if (HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000) != HAL_OK) {
            LOG_ERROR << "Transmission failed";
            control = 0;
            return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::TransmissionFailed); // Return TransmissionFailed error
        }
        // Wait for ACK or NACK with a timeout
        uint32_t receivedEvents = 0;
        LOG_DEBUG << "received events before notify" << receivedEvents;
        LOG_DEBUG << "szie " << size;
        if (xTaskNotifyWait(GNSS_RESPONSE, GNSS_RESPONSE, &receivedEvents, portMAX_DELAY) == pdTRUE) {
            // Check if we received a response
            if (receivedEvents & GNSS_RESPONSE) {
                xQueueReceive(gnssQueueHandleGNSSResponse, &rx_buf_from_queue, pdMS_TO_TICKS(100));
                LOG_DEBUG << "received events" << receivedEvents;
                for (uint16_t i = 0; i < 460; i++) {
                    if (rx_buf_from_queue[i] == 131) {
                        // ACK received
                        LOG_DEBUG << "ACK received!";
                        memset(rx_buf_from_queue, 0, 1024);
                        control = 0;
                        return {}; // Success case (void), indicating ACK received
                    }
                    if (rx_buf_from_queue[i] == 132) {
                        // NACK received
                        LOG_ERROR << "NACK received!";
                        memset(rx_buf_from_queue, 0, 1024);
                        control = 0;
                        return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::NACKReceived); // Return NACKReceived error
                    }
                }
            }
        }
    }
    // If we exit the loop without success, return a Timeout error
    LOG_ERROR << "Failed to receive ACK/NACK after maximum retries.";
    memset(rx_buf_from_queue, 0, 1024);
    control = 0;
    return etl::unexpected<ErrorFromGNSS>(ErrorFromGNSS::Timeout);
}

void GNSSTask::initQueuesToAcceptPointers() {
    gnssQueueHandleDefault = xQueueCreate(uxQueueLength, sizeof(uint8_t*));
    if (gnssQueueHandleDefault == nullptr)
        LOG_ERROR << "Queue did not initialized properly";
    else {
        LOG_INFO << "Queue initialized with success";
    }
    gnssQueueHandleGNSSResponse = xQueueCreate(uxQueueLength, sizeof(uint8_t*));
    if (gnssQueueHandleGNSSResponse == nullptr)
        LOG_ERROR << "Queue did not initialized properly";
    else {
        LOG_INFO << "Queue initialized with success";
    }
}

void GNSSTask::execute() {
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_RSTN_GPIO_Port, GNSS_RSTN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);

    rx_buf_pointer = rx_buf;
    startReceiveFromUARTwithIdle(rx_buf_pointer, 1024);
    vTaskDelay(10);
    controlGNSSwithNotify(GNSSReceiver::setFactoryDefaults(DefaultType::RebootAfterSettingToFactoryDefaults));

    uint32_t receivedEvents = 0;
    uint8_t counter = 0;
    uint8_t* rx_buf_from_queue = nullptr;

    while (true) {
        LOG_DEBUG << size;
        xTaskNotifyWait(GNSS_MESSAGE_READY, GNSS_MESSAGE_READY, &receivedEvents, portMAX_DELAY);
        if (receivedEvents & GNSS_MESSAGE_READY) {
            xQueueReceive(gnssQueueHandleDefault, &rx_buf_from_queue, pdMS_TO_TICKS(100));
            if (rx_buf_from_queue != nullptr) {
                printing(rx_buf_from_queue);
                counter++;
            }
        }
        if (counter == 4) {
            if (counter == 4) {
                counter = 0;
                switchGNSSMode();
            }
        }
    }
}
