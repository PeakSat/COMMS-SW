#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"


void GNSSTask::printing() {
    for (uint16_t i = 0; i < size; i++) {
        GNSSMessage.push_back(incomingMessage[i]);
    }
    LOG_DEBUG << GNSSMessage.c_str();
    // Clear the previous data
    GNSSMessage.clear();
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

    while (true) {
        if (gnss_flag) {
            gnss_flag = false;
            printing_counter++;
            if (printing_counter == 5) {
                printing();
                printing_counter = 0;
            }
        }
        vTaskDelay(10);
    }
}
