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
        for (uint16_t i = 0; i < size; i++) {
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

void GNSSTask::changeIntervalofNMEAStrings(etl::vector<etl::string<3>, 10>& nmeaStrings, uint8_t interval, Attributes attributes) {
    interval = nmea_string_interval;
    for (auto& str: nmeaStrings)
        GNSSTask::controlGNSS(gnssReceiver.configureNMEAStringInterval(str, interval, GNSSDefinitions::Attributes::UpdateToSRAM));
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
        vTaskDelay(50);
        // Check if data is available in UART
        if (ack_flag) {
            // ACK received
            ack_flag = false;
            LOG_INFO << "ACK received!";
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

void GNSSTask::execute() {

    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_RSTN_GPIO_Port, GNSS_RSTN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, this->incomingMessage, 512);
    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
    //  disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);

    vTaskDelay(50);

    etl::vector<etl::string<3>, 10> nmeaStrings;
    initializeNMEAStrings(nmeaStrings);

    changeIntervalofNMEAStrings(nmeaStrings, 10, GNSSDefinitions::Attributes::UpdateToSRAM);

    while (true) {
        xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
        if (size == 460)
            printing();
    }
}