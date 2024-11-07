#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"


extern uint8_t receivedByte;
extern bool controlGNSSenabled;
extern bool ack_flag;
extern bool nack_flag;
extern bool uart_flag_it;

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
    controlGNSSenabled = true;
    for (uint8_t byte: gnssMessageToSend.messageBody)
        LOG_DEBUG << byte;

    vTaskDelay(50);
    HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000);

    // Wait for notification with a timeout (e.g., 1000 ms)
    uint32_t receivedValue = 0;
    xTaskNotifyWait(0, 0, &receivedValue, 1000);

    if (receivedValue == 1) {
        // ACK received
        LOG_INFO << "ACK received!";
        // Handle ACK case, proceed with the next operation
    } else if (receivedValue == 2) {
        // NACK received
        LOG_ERROR << "NACK received!";
        // Handle NACK case (retry or report failure)
    } else {
        // Timeout or unexpected response
        LOG_ERROR << "Timeout or unexpected response received!";
        // Handle timeout: retry, report failure, or handle error
    }
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

    controlGNSS(gnssReceiver.configureNMEATalkerID(GNSSDefinitions::TalkerIDType::GPMode, GNSSDefinitions::Attributes::UpdateSRAMandFLASH));
    //        controlGNSS(gnssReceiver.setFactoryDefaults(GNSSDefinitions::DefaultType::Reserved));
    //        controlGNSS(gnssReceiver.querySoftwareVersion(GNSSDefinitions::SoftwareType::SystemCode));
    //        controlGNSS(gnssReceiver.queryInterferenceDetectionStatus());
    //            controlGNSS(gnssReceiver.configureMessageType(GNSSDefinitions::ConfigurationType::NMEA, GNSSDefinitions::Attributes::UpdateToSRAM));
    //    controlGNSS(gnssReceiver.configureGNSSNavigationMode(GNSSDefinitions::NavigationMode::Airborne, GNSSDefinitions::Attributes::UpdateToSRAM));
    //        controlGNSS(gnssReceiver.configureSystemPositionRate(GNSSDefinitions::PositionRate::Option2Hz, GNSSDefinitions::Attributes::UpdateToSRAM));
    //        controlGNSS(gnssReceiver.query1PPSTiming());
    etl::vector<etl::string<3>, 10> nmeaStrings;
    initializeNMEAStrings(nmeaStrings);
    //    changeIntervalofNMEAStrings(nmeaStrings, 10, GNSSDefinitions::Attributes::UpdateToSRAM);
    //        controlGNSS(gnssReceiver.setFactoryDefaults(GNSSDefinitions::DefaultType::Reserved));
    //        controlGNSS(gnssReceiver.configureSystemPowerMode(GNSSDefinitions::PowerMode::PowerSave, GNSSDefinitions::Attributes::UpdateToSRAM));

    while (true) {
        xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
        printing();

        //        if (ack_flag) {
        //            LOG_DEBUG << "ACKKKK";
        //            ack_flag = false;
        //        }
        //        if (nack_flag) {
        //            LOG_DEBUG << "NNNNNNNNNACKKKK";
        //            nack_flag = false;
        //        }
        //        if (uart_flag_it) {
        //            LOG_DEBUG << "got into IT";
        //            uart_flag_it = false;
        //        }
    }
}