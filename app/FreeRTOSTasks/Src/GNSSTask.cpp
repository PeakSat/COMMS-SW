#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"


void GNSSTask::printing() {
    printing_counter++;
    if (printing_counter == 1) {
        printing_counter = 0;
        for (uint16_t i = 0; i < size; i++) {
            GNSSMessageString.push_back(incomingMessage[i]);
        }
        LOG_DEBUG << GNSSMessageString.c_str();
        // Clear the previous data
        GNSSMessageString.clear();
    }
}

void GNSSTask::controlGNSS(GNSSMessage gnssMessageToSend) {
    for (uint8_t byte: gnssMessageToSend.messageBody)
        LOG_DEBUG << byte;
    vTaskDelay(50);
    HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000);
}

void GNSSTask::sendDataToGNSS() {
    GNSSMessage gnssMessage;
    gnssMessage = gnssReceiver.configureNMEATalkerID(GNSSDefinitions::TalkerIDType::GPMode, GNSSDefinitions::Attributes::UpdateSRAMandFLASH);
    //    gnssMessage = gnssReceiver.configureNMEATalkerID(GNSSDefinitions::TalkerIDType::GNMode,GNSSDefinitions::Attributes::UpdateSRAMandFLASH);
    //    gnssMessage = gnssReceiver.setFactoryDefaults();
    LOG_DEBUG << "the message that will be send";
    for (uint8_t byte: gnssMessage.messageBody)
        LOG_DEBUG << byte;
    HAL_UART_Transmit(&huart5, gnssMessage.messageBody.data(), gnssMessage.messageBody.size(), 1000);
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

    //    controlGNSS(gnssReceiver.configureNMEATalkerID(GNSSDefinitions::TalkerIDType::GPMode,GNSSDefinitions::Attributes::UpdateSRAMandFLASH));
    //    controlGNSS(gnssReceiver.setFactoryDefaults(GNSSDefinitions::DefaultType::Reserved));
    //    controlGNSS(gnssReceiver.querySoftwareVersion(GNSSDefinitions::SoftwareType::SystemCode));
    //    controlGNSS(gnssReceiver.configureSystemPositionRate(static_cast<uint8_t>(GNSSDefinitions::InterferenceDetectControl::Enable), GNSSDefinitions::Attributes::UpdateToSRAM));
    //    controlGNSS(gnssReceiver.queryInterferenceDetectionStatus());
    //    controlGNSS(gnssReceiver.configureMessageType(GNSSDefinitions::ConfigurationType::NMEA, GNSSDefinitions::Attributes::UpdateToSRAM));
    //    controlGNSS(gnssReceiver.configureGNSSNavigationMode(GNSSDefinitions::NavigationMode::Airborne, GNSSDefinitions::Attributes::UpdateToSRAM));
    //    controlGNSS(gnssReceiver.configureSystemPositionRate(GNSSDefinitions::PositionRate::Option2Hz, GNSSDefinitions::Attributes::UpdateToSRAM));
    //    controlGNSS(gnssReceiver.query1PPSTiming());
    controlGNSS(gnssReceiver.configureNMEAStringInterval(GNSSDefinitions::)
    while (true) {
        xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
        printing();
        for (uint16_t i = 0; i < size; i++) {
            if (incomingMessage[i] == 132) {
                LOG_DEBUG << "NNNNNNNNNNNNNNACK";
            }
            if (incomingMessage[i] == 131)
                LOG_DEBUG << "ACKKKKKKKKKKKKKKKKKKKKKKKKKK";
        }
    }
}
