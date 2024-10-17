#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"


void GNSSTask::execute() {
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_RSTN_GPIO_Port, GNSS_RSTN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);
//    auto message = GNSSReceiver::configureGNSSConstellationTypeForNavigationSolution(0x01, GNSSDefinitions::Attributes::UpdateSRAMandFLASH);
//    HAL_UART_Transmit(&huart5, message.messageBody.data(), message.messageBody.size(), HAL_MAX_DELAY);
//    message = GNSSReceiver::configureGNSSNavigationMode(GNSSDefinitions::NavigationMode::Auto, GNSSDefinitions::Attributes::UpdateSRAMandFLASH);
//    HAL_UART_Transmit(&huart5, message.messageBody.data(), message.messageBody.size(), HAL_MAX_DELAY);
//
//    etl::vector<uint8_t, 12> intervals;
//    intervals.fill(0);
//    intervals[0] = 5;
//
//    message = GNSSReceiver::configureExtendedNMEAMessageInterval(intervals, GNSSDefinitions::Attributes::UpdateSRAMandFLASH);
//    HAL_UART_Transmit(&huart5, message.messageBody.data(), message.messageBody.size(), HAL_MAX_DELAY);
//
//    message = GNSSReceiver::configureDOPMask(GNSSDefinitions::DOPModeSelect::AutoMode, 30, 30, 30, GNSSDefinitions::Attributes::UpdateSRAMandFLASH);
//    HAL_UART_Transmit(&huart5, message.messageBody.data(), message.messageBody.size(), HAL_MAX_DELAY);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, incomingMessage, 512);
    while(true){
//        LOG_DEBUG << "{START OF" << this->TaskName << "}" ;
        xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
        vTaskDelay(15000);
        uint8_t found_dollar = 0;
        etl::string<512> GNSSMessage = {};
        for (uint8_t  byte : incomingMessage) {
            GNSSMessage.push_back(byte);
        }
        LOG_DEBUG << GNSSMessage.c_str();
        new(&(incomingMessage)) uint8_t[512];
//        LOG_DEBUG << "{END OF" << this->TaskName << "}" ;
    }
}

