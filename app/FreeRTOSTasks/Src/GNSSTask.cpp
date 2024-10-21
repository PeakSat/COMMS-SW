#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"


void GNSSTask::execute() {

    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_RSTN_GPIO_Port, GNSS_RSTN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, this->incomingMessage, 512);
    while(true){
        LOG_DEBUG << "GNSS TASK";
        vTaskDelay(5000);
        uint8_t found_dollar = 0;
        etl::string<512> GNSSMessage = {};
        for (uint8_t  byte : incomingMessage) {
            GNSSMessage.push_back(byte);
        }
        LOG_DEBUG << GNSSMessage.c_str();
        new(&(incomingMessage)) uint8_t[512];

    }
}

